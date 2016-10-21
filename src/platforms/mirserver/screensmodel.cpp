/*
 * Copyright (C) 2015-2016 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "screensmodel.h"

#include "screenwindow.h"
#include "qtcompositor.h"
#include "logging.h"
#include "mirserverintegration.h"
#include "screen.h"

// Mir
#include <mir/graphics/display.h>
#include <mir/graphics/display_buffer.h>

// Qt
#include <QScreen>
#include <QQuickWindow>
#include <qpa/qwindowsysteminterface.h>
#include <QGuiApplication> // for qApp

// std
#include <memory>

namespace mg = mir::graphics;

#define DEBUG_MSG qCDebug(QTMIR_SCREENS).nospace() << "ScreensModel[" << this <<"]::" << __func__

ScreensModel::ScreensModel(QObject *parent)
    : QObject(parent)
    , m_compositing(false)
{
    DEBUG_MSG << "()";
}

// init only after MirServer has initialized - runs on MirServerThread!!!
void ScreensModel::init(const std::shared_ptr<mir::graphics::Display> &display,
                        const std::shared_ptr<mir::compositor::Compositor> &compositor)
{
    m_display = display;
    m_compositor = compositor;

    // Use a Blocking Queued Connection to enforce synchronization of Qt GUI thread with Mir thread(s)
    // on compositor shutdown. Compositor startup can be lazy.
    // Queued connections work because the thread affinity of this class is with the Qt GUI thread.
    auto qtCompositor = static_cast<QtCompositor *>(compositor.get());
    connect(qtCompositor, &QtCompositor::starting,
            this, &ScreensModel::onCompositorStarting);
    connect(qtCompositor, &QtCompositor::stopping,
            this, &ScreensModel::onCompositorStopping, Qt::BlockingQueuedConnection);
}

// terminate before shutting down the Mir server, or else liable to deadlock with the blocking connection above
// Runs on MirServerThread!!!
void ScreensModel::terminate()
{
    auto qtCompositor = static_cast<QtCompositor *>(m_compositor.get());
    qtCompositor->disconnect();
}

void ScreensModel::onCompositorStarting()
{
    DEBUG_MSG << "()";
    m_compositing = true;

    update();

    // (Re)Start Qt's render thread by setting all windows with a corresponding screen to exposed.
    allWindowsSetExposed(true);
}

void ScreensModel::onCompositorStopping()
{
    DEBUG_MSG << "()";
    m_compositing = false;

    // Stop Qt's render threads by setting all its windows it obscured. Must
    // block until all windows have their GL contexts released.
    allWindowsSetExposed(false);

    update();
}

void ScreensModel::update()
{
    DEBUG_MSG << "()";
    auto display = m_display.lock();
    if (!display)
        return;
    auto displayConfig = display->configuration();

    // Mir only tells us something changed, it is up to us to figure out what.
    QList<Screen*> newScreenList;
    QList<Screen*> oldScreenList = m_screenList;
    QHash<ScreenWindow*, Screen*> windowMoveList;
    m_screenList.clear();

    displayConfig->for_each_output(
        [this, &oldScreenList, &newScreenList, &windowMoveList](const mg::DisplayConfigurationOutput &output) {
            if (output.used && output.connected) {
                Screen *screen = findScreenWithId(oldScreenList, output.id);
                if (screen) { // we've already set up this display before

                    // Can we re-use the existing Screen?
                    if (canUpdateExistingScreen(screen, output)) {
                        screen->setMirDisplayConfiguration(output);
                        oldScreenList.removeAll(screen);
                        m_screenList.append(screen);
                    } else {
                        // no, need to delete it and re-create with new config
                        DEBUG_MSG << "() - recreating screen for output=" << output.id.as_value();

                        auto newScreen = createScreen(output);
                        newScreenList.append(newScreen);

                        // if Window on this Screen, arrange to move it to the new Screen
                        Q_FOREACH (ScreenWindow* window, screen->windows()) {
                            windowMoveList.insert(window, newScreen);
                        }
                        m_screenList.append(newScreen);
                    }
                } else {
                    DEBUG_MSG << "() - new screen for output=" << output.id.as_value();
                    // new display, so create Screen for it
                    screen = createScreen(output);
                    newScreenList.append(screen);
                    m_screenList.append(screen);
                }
            }
        }
    );

    // Announce new Screens to Qt
    Q_FOREACH (auto screen, newScreenList) {
        Q_EMIT screenAdded(screen);
    }

    // Move Windows from about-to-be-deleted Screens to new Screen
    auto i = windowMoveList.constBegin();
    while (i != windowMoveList.constEnd()) {
        DEBUG_MSG << "() - moving platform window " << i.key() <<  " from " << static_cast<Screen*>(i.key()->screen()) << " to " << i.value();
        i.key()->setScreen(i.value());
        i++;
    }

    // Delete any old & unused Screens
    Q_FOREACH (auto screen, oldScreenList) {
        DEBUG_MSG << "() - removed Screen with id " << screen->m_outputId.as_value()
                               << " and geometry " << screen->geometry();

        Q_FOREACH (ScreenWindow* window, screen->windows()) {
            if (window->window() && window->isExposed()) {
                window->window()->hide();
            }
        }
        bool ok = QMetaObject::invokeMethod(qApp, "onScreenAboutToBeRemoved", Qt::DirectConnection, Q_ARG(QScreen*, screen->screen()));
        if (!ok) {
            DEBUG_MSG << "() - Failed to invoke QGuiApplication::onScreenAboutToBeRemoved(QScreen*) slot.";
        }
        Q_EMIT screenRemoved(screen); // should delete the backing Screen
    }

    // Match up the new Mir DisplayBuffers with each Screen
    display->for_each_display_sync_group([&](mg::DisplaySyncGroup &group) {
        group.for_each_display_buffer([&](mg::DisplayBuffer &buffer) {
            // only way to match Screen to a DisplayBuffer is by matching the geometry
            QRect dbGeom(buffer.view_area().top_left.x.as_int(),
                         buffer.view_area().top_left.y.as_int(),
                         buffer.view_area().size.width.as_int(),
                         buffer.view_area().size.height.as_int());

            Q_FOREACH (auto screen, m_screenList) {
                if (dbGeom == screen->geometry()) {
                    screen->setMirDisplayBuffer(&buffer, &group);
                    break;
                }
            }
        });
    });

    qCDebug(QTMIR_SCREENS) << "=======================================";
    Q_FOREACH (auto screen, m_screenList) {
        qCDebug(QTMIR_SCREENS) << screen << "- id:" << screen->m_outputId.as_value()
                               << "geometry:" << screen->geometry()
                               << "window:" << screen->primaryWindow()
                               << "type:" << screen->name()
                               << "scale:" << screen->scale();
    }
    qCDebug(QTMIR_SCREENS) << "=======================================";
}

bool ScreensModel::canUpdateExistingScreen(const Screen *screen, const mg::DisplayConfigurationOutput &output)
{
    // Compare the properties of the existing Screen with its new configuration. Properties
    // like geometry, refresh rate and dpi can be updated on existing screens. Other property
    // changes cannot be applied to existing screen, so will need to delete existing Screen and
    // create new Screen with new properties.
    bool canUpdateExisting = true;

    if (!qFuzzyCompare(screen->scale(), output.scale)) {
        canUpdateExisting = true; //false; FIXME
    }

    return canUpdateExisting;
}

void ScreensModel::allWindowsSetExposed(bool exposed)
{
    Q_FOREACH (const auto screen, m_screenList) {
        Q_FOREACH (ScreenWindow* window, screen->windows()) {
            window->setExposed(exposed);
        }
    }
}

Screen* ScreensModel::createScreen(const mg::DisplayConfigurationOutput &output) const
{
    return new Screen(output);
}

Screen* ScreensModel::findScreenWithId(const QList<Screen *> &list, const mg::DisplayConfigurationOutputId id)
{
    for (Screen *screen : list) {
        if (screen->m_outputId == id) {
            return screen;
        }
    }
    return nullptr;
}

QWindow* ScreensModel::getWindowForPoint(QPoint point) //FIXME - not thread safe & not efficient
{
    // This is a part optimization, and a part work-around for AP generated input events occasionally
    // appearing outside the screen borders: https://bugs.launchpad.net/qtmir/+bug/1508415
    if (m_screenList.length() == 1) {
        auto primaryWindow = m_screenList.first()->primaryWindow();
        if (primaryWindow) return primaryWindow->window();
    }

    Q_FOREACH (Screen *screen, m_screenList) {
        if (screen->primaryWindow() && screen->geometry().contains(point)) {
            return screen->primaryWindow()->window();
        }
    }
    return nullptr;
}
