/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include "screencontroller.h"

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

Q_LOGGING_CATEGORY(QTMIR_SCREENS, "qtmir.screens")

namespace mg = mir::graphics;


ScreenController::ScreenController(QObject *parent)
    : QObject(parent)
    , m_compositing(false)
{
    qCDebug(QTMIR_SCREENS) << "ScreenController::ScreenController";
}

// init only after MirServer has initialized - runs on MirServerThread!!!
void ScreenController::init(const std::shared_ptr<mir::graphics::Display> &display,
                            const std::shared_ptr<mir::shell::DisplayConfigurationController> &controller,
                            const std::shared_ptr<mir::compositor::Compositor> &compositor)
{
    m_display = display;
    m_displayConfigurationController = controller;
    m_compositor = compositor;

    // Use a Blocking Queued Connection to enforce synchronization of Qt GUI thread with Mir thread(s)
    // on compositor shutdown. Compositor startup can be lazy.
    // Queued connections work because the thread affinity of this class is with the Qt GUI thread.
    auto qtCompositor = static_cast<QtCompositor *>(compositor.get());
    connect(qtCompositor, &QtCompositor::starting,
            this, &ScreenController::onCompositorStarting);
    connect(qtCompositor, &QtCompositor::stopping,
            this, &ScreenController::onCompositorStopping, Qt::BlockingQueuedConnection);
}

// terminate before shutting down the Mir server, or else liable to deadlock with the blocking connection above
// Runs on MirServerThread!!!
void ScreenController::terminate()
{
    auto qtCompositor = static_cast<QtCompositor *>(m_compositor.get());
    qtCompositor->disconnect();
}

void ScreenController::onCompositorStarting()
{
    qCDebug(QTMIR_SCREENS) << "ScreenController::onCompositorStarting";
    m_compositing = true;

    update();

    // (Re)Start Qt's render thread by setting all windows with a corresponding screen to exposed.
    for (auto screen : m_screenList) {
        auto window = static_cast<ScreenWindow *>(screen->window());
        if (window && window->window()) {
            window->setExposed(true);
        }
    }
}

void ScreenController::onCompositorStopping()
{
    qCDebug(QTMIR_SCREENS) << "ScreenController::onCompositorStopping";
    m_compositing = false;

    // Stop Qt's render threads by setting all its windows it obscured. Must
    // block until all windows have their GL contexts released.
    for (auto screen : m_screenList) {
        auto window = static_cast<ScreenWindow *>(screen->window());
        if (window && window->window()) {
            window->setExposed(false);
        }
    }

    update();
}

void ScreenController::update()
{
    qCDebug(QTMIR_SCREENS) << "ScreenController::update";
    m_configurationQueue.clear();
    auto display = m_display.lock();
    if (!display)
        return;
    auto displayConfig = display->configuration();

    // Mir only tells us something changed, it is up to us to figure out what.
    QList<Screen*> newScreenList;
    QList<Screen*> oldScreenList = m_screenList;
    m_screenList.clear();

    displayConfig->for_each_output(
        [this, &oldScreenList, &newScreenList](const mg::DisplayConfigurationOutput &output) {
            if (output.used && output.connected) {
                Screen *screen = findScreenWithId(oldScreenList, output.id);
                if (screen) { // we've already set up this display before, refresh its internals
                    screen->setMirDisplayConfiguration(output);
                    oldScreenList.removeAll(screen);
                } else {
                    // new display, so create Screen for it
                    screen = this->createScreen(output);
                    newScreenList.append(screen);
                    qCDebug(QTMIR_SCREENS) << "Added Screen with id" << output.id.as_value()
                                           << "and geometry" << screen->geometry();
                }
                m_screenList.append(screen);
            }
        }
    );

    // Delete any old & unused Screens
    for (auto screen: oldScreenList) {
        qCDebug(QTMIR_SCREENS) << "Removed Screen with id" << screen->m_outputId.as_value()
                               << "and geometry" << screen->geometry();
        // The screen is automatically removed from Qt's internal list by the QPlatformScreen destructor.
        auto window = static_cast<ScreenWindow *>(screen->window());
        if (window && window->window() && window->isExposed()) {
            window->window()->hide();
        }
        bool ok = QMetaObject::invokeMethod(qApp, "onScreenAboutToBeRemoved", Qt::DirectConnection, Q_ARG(QScreen*, screen->screen()));
        if (!ok) {
            qCWarning(QTMIR_SCREENS) << "Failed to invoke QGuiApplication::onScreenAboutToBeRemoved(QScreen*) slot.";
        }
        delete screen;
    }

    // Match up the new Mir DisplayBuffers with each Screen
    display->for_each_display_sync_group([&](mg::DisplaySyncGroup &group) {
        group.for_each_display_buffer([&](mg::DisplayBuffer &buffer) {
            // only way to match Screen to a DisplayBuffer is by matching the geometry
            QRect dbGeom(buffer.view_area().top_left.x.as_int(),
                         buffer.view_area().top_left.y.as_int(),
                         buffer.view_area().size.width.as_int(),
                         buffer.view_area().size.height.as_int());

            for (auto screen : m_screenList) {
                if (dbGeom == screen->geometry()) {
                    screen->setMirDisplayBuffer(&buffer, &group);
                    break;
                }
            }
        });
    });

    qCDebug(QTMIR_SCREENS) << "=======================================";
    for (auto screen: m_screenList) {
        qCDebug(QTMIR_SCREENS) << screen << "- id:" << screen->m_outputId.as_value()
                               << "geometry:" << screen->geometry()
                               << "window:" << screen->window()
                               << "type" << static_cast<int>(screen->outputType());
    }
    qCDebug(QTMIR_SCREENS) << "=======================================";

    for (auto screen : newScreenList) {
        Q_EMIT screenAdded(screen);
    }
}

Screen* ScreenController::createScreen(const mir::graphics::DisplayConfigurationOutput &output) const
{
    return new Screen(output);
}

Screen* ScreenController::findScreenWithId(const QList<Screen *> &list, const mg::DisplayConfigurationOutputId id)
{
    for (Screen *screen : list) {
        if (screen->m_outputId == id) {
            return screen;
        }
    }
    return nullptr;
}

QWindow* ScreenController::getWindowForPoint(const QPoint &point) //FIXME - not thread safe & not efficient
{
    // This is a part optimization, and a part work-around for AP generated input events occasionally
    // appearing outside the screen borders: https://bugs.launchpad.net/qtmir/+bug/1508415
    if (m_screenList.length() == 1 && m_screenList.first()->window()) {
        return m_screenList.first()->window()->window();
    }

    for (Screen *screen : m_screenList) {
        if (screen->window() && screen->geometry().contains(point)) {
            return screen->window()->window();
        }
    }
    return nullptr;
}

CustomScreenConfiguration ScreenController::getConfigurationFor(Screen *screen)
{
    CustomScreenConfiguration config {
        screen->outputId(),
        screen->geometry().topLeft(),
        screen->currentModeIndex(),
        screen->powerMode(),
        mir_orientation_normal, //screen->orientation(),
        screen->scale(),
        screen->formFactor()
    };
    return config;
}

void ScreenController::queueConfigurationChange(CustomScreenConfiguration config)
{
    m_configurationQueue.append(config);
}

void ScreenController::applyConfigurationChanges()
{
    auto controller = m_displayConfigurationController.lock();
    auto display = m_display.lock();

    if (m_configurationQueue.isEmpty() || !controller || !display) {
        return;
    }

    auto displayConfiguration = display->configuration();

    Q_FOREACH(auto config, m_configurationQueue) {
        displayConfiguration->for_each_output(
            [&config](mg::UserDisplayConfigurationOutput& outputConfig)
            {
                if (config.id == outputConfig.id) {
                    outputConfig.current_mode_index = config.modeIndex;
                    outputConfig.power_mode = config.powerMode;
                    outputConfig.scale = config.scale;
                    outputConfig.form_factor = config.formFactor;
                }
            });
    }

    controller->set_base_configuration(std::move(displayConfiguration)).get();
}
