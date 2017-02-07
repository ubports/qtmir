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

#ifndef SCREENCONTROLLER_H
#define SCREENCONTROLLER_H

#include "mirdisplayconfigurationobserver.h"

#include <QObject>
#include <QPoint>

// Mir
#include <mir/graphics/display_configuration.h>

// std
#include <memory>

namespace mir {
    namespace graphics { class Display; }
    namespace compositor { class DisplayListener; }
}
class PlatformScreen;
class QWindow;
class QtCompositor;

/*
 * ScreensModel monitors the Mir display configuration and compositor status, and updates
 * the relevant QScreen and QWindow states accordingly.
 *
 * Primary purposes are:
 * 1. to update QScreen state on Mir display configuration changes
 * 2. to stop the Qt renderer by hiding its QWindow when Mir wants to stop all compositing,
 *    and resume Qt's renderer by showing its QWindow when Mir wants to resume compositing.
 *
 *
 * Threading Note:
 * This object must have affinity to the main Qt GUI thread, as it creates & destroys Platform
 * objects which Qt uses internally. However beware as the init() & terminate() methods need to
 * be called on the MirServerThread thread, as we need to monitor the screen state *after*
 * Mir has initialized but before Qt's event loop has started, and tear down before Mir terminates.
 * Also note the MirServerThread does not have an QEventLoop.
 *
 * All other methods must be called on the Qt GUI thread.
 */

class ScreensModel : public MirDisplayConfigurationObserver
{
    Q_OBJECT
public:
    explicit ScreensModel(QObject *parent = 0);

    QList<PlatformScreen*> screens() const { return m_screenList; }
    bool compositing() const { return m_compositing; }

    QWindow* getWindowForPoint(QPoint point);

Q_SIGNALS:
    void screenAdded(PlatformScreen *screen);
    void screenRemoved(PlatformScreen *screen);

public Q_SLOTS:
    void update();

public:
    // called by MirServer
    void init(
        const std::shared_ptr<mir::graphics::Display>& display,
        const std::shared_ptr<QtCompositor>& compositor,
        const std::shared_ptr<mir::compositor::DisplayListener>& displayListener);
    void terminate();

    // override for testing purposes
    virtual PlatformScreen *createScreen(const mir::graphics::DisplayConfigurationOutput &output) const;

protected Q_SLOTS:
    void onCompositorStarting();
    void onCompositorStopping();

private:
    PlatformScreen* findScreenWithId(const QList<PlatformScreen*> &list, const mir::graphics::DisplayConfigurationOutputId id);
    bool canUpdateExistingScreen(const PlatformScreen *screen, const mir::graphics::DisplayConfigurationOutput &output);
    void allWindowsSetExposed(bool exposed);

    std::weak_ptr<mir::graphics::Display> m_display;
    std::shared_ptr<QtCompositor> m_compositor;
    std::shared_ptr<mir::compositor::DisplayListener> m_displayListener;
    QList<PlatformScreen*> m_screenList;
    bool m_compositing;
};

#endif // SCREENCONTROLLER_H
