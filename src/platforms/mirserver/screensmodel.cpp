/*
 * Copyright (C) 2015-2017 Canonical, Ltd.
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

#include "logging.h"
#include "mirqtconversion.h"
#include "mirserverintegration.h"
#include "qtcompositor.h"
#include "screen.h"
#include "screenwindow.h"
#include "orientationsensor.h"

// Mir
#include <mir/graphics/display.h>
#include <mir/graphics/display_buffer.h>
#include <mir/graphics/display_configuration.h>
#include <mir/compositor/display_listener.h>

// Qt
#include <QScreen>
#include <QGuiApplication> // for qApp
#include <qpa/qwindowsysteminterface.h>

// std
#include <memory>

namespace mg = mir::graphics;

ScreensModel::ScreensModel(QObject *parent)
    : QObject(parent)
    , m_compositing(false)
    , m_orientationSensor(std::make_shared<OrientationSensor>(this))
{
    qCDebug(QTMIR_SCREENS) << "ScreensModel::ScreensModel";
}

// init only after MirServer has initialized - runs on MirServerThread!!!
void ScreensModel::init(
    const std::shared_ptr<mir::graphics::Display>& display,
    const std::shared_ptr<QtCompositor>& compositor,
    const std::shared_ptr<mir::compositor::DisplayListener>& displayListener)
{
    m_display = display;
    m_compositor = compositor;
    m_displayListener = displayListener;

    // Use a Blocking Queued Connection to enforce synchronization of Qt GUI thread with Mir thread(s)
    // on compositor shutdown. Compositor startup can be lazy.
    // Queued connections work because the thread affinity of this class is with the Qt GUI thread.
    auto qtCompositor = compositor.get();
    connect(qtCompositor, &QtCompositor::starting,
            this, &ScreensModel::onCompositorStarting);
    connect(qtCompositor, &QtCompositor::stopping,
            this, &ScreensModel::onCompositorStopping, Qt::BlockingQueuedConnection);
}

// terminate before shutting down the Mir server, or else liable to deadlock with the blocking connection above
// Runs on MirServerThread!!!
void ScreensModel::terminate()
{
    m_compositor->disconnect();
}

void ScreensModel::onCompositorStarting()
{
    qCDebug(QTMIR_SCREENS) << "ScreensModel::onCompositorStarting";

    if (!havePendingScreenEvents()) {
        m_orientationSensor->start();
        startRenderer();
    }
}

void ScreensModel::onCompositorStopping()
{
    qCDebug(QTMIR_SCREENS) << "ScreensModel::onCompositorStopping";

    m_orientationSensor->stop();

    haltRenderer();
}

bool ScreensModel::havePendingScreenEvents()
{
     auto display = m_display.lock();
     if (!display)
         return false;
    auto displayConfig = display->configuration();

    QList<int> oldScreenIds;
    for (auto screen : m_screenList) {
        oldScreenIds.append(screen->outputId().as_value());
    }

    QList<int> newScreenIds;
    displayConfig->for_each_output(
        [&newScreenIds](const mg::DisplayConfigurationOutput &output) {
            if (output.used && output.connected) {
                newScreenIds.append(output.id.as_value());
            }
    });

    qSort(oldScreenIds);
    qSort(newScreenIds);

    bool have = oldScreenIds != newScreenIds;

    qWarning() << "ScreensModel::havePendingScreenEvents() = " << have;

    return have;
}

void ScreensModel::screenCreated(const miral::Output &output)
{
    QMutexLocker locker(&m_mutex);

    if (m_compositing) {
        if (!QMetaObject::invokeMethod(this, "onCompositorStopping", Qt::BlockingQueuedConnection)) {
            qCWarning(QTMIR_SCREENS) << "Failed to invoke halt render slot.";
        }
    }

    qCDebug(QTMIR_SCREENS) << "ScreensModel::createScreen";

    Screen *screen = createScreen(output);
    m_screenList.append(screen);

    updateBuffers();

    Q_EMIT screenAdded(screen);
    m_displayListener->add_display(qtmir::toMirRectangle(screen->geometry()));

    qCDebug(QTMIR_SCREENS) << "Added Screen with id" << screen->outputId().as_value()
                            << "and geometry" << screen->geometry();

    raport();

    if (!QMetaObject::invokeMethod(this, "onCompositorStarting", Qt::QueuedConnection)) {
        qCWarning(QTMIR_SCREENS) << "Failed to invoke start render slot.";
    }
}

void ScreensModel::deleteScreen(Screen *screen)
{
    qCDebug(QTMIR_SCREENS) << "ScreensModel::deleteScreen" << screen;

    auto window = static_cast<ScreenWindow *>(screen->window());

    // Stop rendering of this window
    if (window && window->window() && window->isExposed()) {
        window->window()->hide();
    }

    m_displayListener->remove_display(qtmir::toMirRectangle(screen->geometry()));

    qCDebug(QTMIR_SCREENS) << "Removing Screen " << screen
                           << "with id: " << screen->outputId().as_value();

    m_screenList.removeAll(screen);
    Q_EMIT screenRemoved(screen); // should delete the backing Screen

    raport();

    updateBuffers();
}

void ScreensModel::screenDeleted(const miral::Output &output)
{
    QMutexLocker locker(&m_mutex);

    qCDebug(QTMIR_SCREENS) << "ScreensModel::deleteScreen(miral)";

    if (m_compositing) {
        if (!QMetaObject::invokeMethod(this, "onCompositorStopping", Qt::BlockingQueuedConnection)) {
            qCWarning(QTMIR_SCREENS) << "Failed to invoke halt render slot.";
        }
    }

    Screen *screen = findScreenWithOutput(m_screenList, output);
    if (screen) {
        deleteScreen(screen);
    } else {
        qWarning(QTMIR_SCREENS) << "Could not find screen to delete!";
    }

    if (!QMetaObject::invokeMethod(this, "onCompositorStarting", Qt::QueuedConnection)) {
        qCWarning(QTMIR_SCREENS) << "Failed to invoke start render slot.";
    }
}

void ScreensModel::screenUpdated(miral::Output const& updated, miral::Output const& original)
{
    QMutexLocker locker(&m_mutex);

    qCDebug(QTMIR_SCREENS) << "ScreensModel::updateScreen";

    if (m_compositing) {
        if (!QMetaObject::invokeMethod(this, "onCompositorStopping", Qt::BlockingQueuedConnection)) {
            qCWarning(QTMIR_SCREENS) << "Failed to invoke halt render slot.";
        }
    }

    Screen *screen = findScreenWithOutput(m_screenList, original);

    // Can we re-use the existing Screen?
    if (canUpdateExistingScreen(screen, original)) {
        screen->setMirDisplayConfiguration(updated);
    } else {
        // no, need to delete it and re-create with new config
        auto newScreen = createScreen(updated);
        qCDebug(QTMIR_SCREENS) << "Need to delete & re-create Screen with id" << newScreen->outputId().as_value()
                                << "and geometry" << newScreen->geometry();

        // if Window on this Screen, arrange to move it to the new Screen
        if (screen->window()) {
            qCDebug(QTMIR_SCREENS) << "Moving ScreenWindow" << screen->window() << "from" << screen << "to" << newScreen;
            newScreen->setWindow(screen->window());
        }
        m_screenList.append(newScreen);
        deleteScreen(screen);
    }

    updateBuffers();
    raport();

    if (!QMetaObject::invokeMethod(this, "onCompositorStarting", Qt::QueuedConnection)) {
        qCWarning(QTMIR_SCREENS) << "Failed to invoke start render slot.";
    }
}

void ScreensModel::raport()
{
    qCDebug(QTMIR_SCREENS) << "=======================================";
    Q_FOREACH (auto screen, m_screenList) {
        qCDebug(QTMIR_SCREENS) << screen << "- id:" << screen->outputId().as_value()
                               << "geometry:" << screen->geometry()
                               << "window:" << screen->window()
                               << "type:" << screen->name()
                               << "scale:" << screen->scale();
    }
    qCDebug(QTMIR_SCREENS) << "=======================================";
}

void ScreensModel::updateBuffers()
{
    auto display = m_display.lock();
    if (!display)
        return;
    auto displayConfig = display->configuration();

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
}

bool ScreensModel::canUpdateExistingScreen(const Screen *screen, const miral::Output &output)
{
    // Compare the properties of the existing Screen with its new configuration. Properties
    // like geometry, refresh rate and dpi can be updated on existing screens. Other property
    // changes cannot be applied to existing screen, so will need to delete existing Screen and
    // create new Screen with new properties.
    bool canUpdateExisting = true;

    if (!qFuzzyCompare(screen->scale(), output.scale())) {
        canUpdateExisting = true; //false; FIXME
    }

    return canUpdateExisting;
}

/*
 * ScreensModel::startRenderer()
 * (Re)Start Qt's render thread by setting all windows with a corresponding screen to exposed.
 * It is asynchronous, it returns before the render thread(s) have started.
 */
void ScreensModel::startRenderer()
{
    Q_FOREACH (const auto screen, m_screenList) {
        // Only set windows exposed on displays which are turned on, as the GL context Mir provided
        // is invalid in that situation
        if (screen->powerMode() == mir_power_mode_on) {
            const auto window = static_cast<ScreenWindow *>(screen->window());
            if (window && window->window()) {
                window->setExposed(true);
            }
        }
    }
    m_compositing = true;
}

/*
 * ScreensModel::haltRenderer()
 * Stop Qt's render thread(s) by setting all windows with a corresponding screen to not exposed.
 * It is blocking, it returns after the render thread(s) have all stopped.
 */
void ScreensModel::haltRenderer()
{
    m_compositing = false;
    Q_FOREACH (const auto screen, m_screenList) {
        const auto window = static_cast<ScreenWindow *>(screen->window());
        if (window && window->window()) {
            window->setExposed(false);
        }
    }
}

Screen* ScreensModel::createScreen(const miral::Output &output) const
{
    return new Screen(output, m_orientationSensor);
}

Screen* ScreensModel::findScreenWithOutput(const QList<Screen *> &list, const miral::Output &output)
{
    for (Screen *screen : list) {
        if (screen->isSameOutput(output)) {
            return screen;
        }
    }
    return nullptr;
}
