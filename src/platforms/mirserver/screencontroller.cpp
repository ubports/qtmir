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
#include <mir/main_loop.h>

// Qt
#include <QScreen>
#include <QQuickWindow>
#include <qpa/qwindowsysteminterface.h>

// std
#include <memory>

Q_LOGGING_CATEGORY(QTMIR_SCREENS, "qtmir.screens")

namespace mg = mir::graphics;


ScreenController::ScreenController(QObject *parent)
    : QObject(parent)
{
    qCDebug(QTMIR_SCREENS) << "ScreenController::ScreenController";
}

// init only after MirServer has initialized - runs on MirServerThread!!!
void ScreenController::init(const std::shared_ptr<mir::graphics::Display> &display,
                            const std::shared_ptr<mir::compositor::Compositor> &compositor,
                            const std::shared_ptr<mir::MainLoop> &mainLoop)
{
    m_display = display;
    m_compositor = compositor;

    // Use a Blocking Queued Connection to enforce synchronization of Qt GUI thread with Mir thread(s)
    // on compositor shutdown. Compositor startup can be lazy.
    // Queued connections work because the thread affinity of this class is with the Qt GUI thread.
    auto qtCompositor = static_cast<QtCompositor *>(compositor.get());
    connect(qtCompositor, &QtCompositor::starting,
            this, &ScreenController::onCompositorStarting);
    connect(qtCompositor, &QtCompositor::stopping,
            this, &ScreenController::onCompositorStopping, Qt::BlockingQueuedConnection);


    display->register_configuration_change_handler(*mainLoop, [this]() {
        // display hardware configuration changed, update! - not called when we set new configuration
        QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
    });
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

Screen* ScreenController::getUnusedScreen()
{
    if (m_screenList.empty()) {
        return nullptr;
    } else if (m_screenList.size() == 1) {
        return m_screenList.at(0);
    }

    // FIXME: Until we have better way of identifying screens, prioritize outputs based on their output type.
    // Note the priorities defined here are nothing more than guesses. It tries to select internal displays first,
    // then digital outputs, and finally analogue.
    QMap <int, Screen*> priorityList;
    auto prioritize = [](const mg::DisplayConfigurationOutputType &type) {
        using out = mg::DisplayConfigurationOutputType;
        switch(type) {
        case out::lvds:
        case out::edp:
            return 0;
        case out::displayport:
        case out::hdmia:
        case out::hdmib:
            return 1;
        case out::dvii:
        case out::dvid:
        case out::dvia:
            return 2;
        case out::vga:
            return 3;
        case out::ninepindin:
            return 4;
        case out::component:
        case out::composite:
        case out::svideo:
            return 5;
        case out::tv:
            return 6;
        case out::unknown:
        default:
            return 9;
        }
    };

    for (auto screen : m_screenList) {
        if (!screen->window()) {
            priorityList.insert(prioritize(screen->outputType()), screen);
        }
    }

    qCDebug(QTMIR_SCREENS) << "Prioritized list of available outputs:" << priorityList;
    return priorityList.first(); // Map sorted by key, so first is the key with highest priority.
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
    for (Screen *screen : m_screenList) {
        if (screen->window() && screen->geometry().contains(point)) {
            return screen->window()->window();
        }
    }
    return nullptr;
}
