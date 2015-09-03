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

#ifndef SCREENCONTROLLER_H
#define SCREENCONTROLLER_H

#include <QObject>
#include <QPoint>

// Mir
#include <mir/graphics/display_configuration.h>

// std
#include <memory>

namespace mir {
    namespace graphics { class Display; }
    namespace compositor { class Compositor; }
}
class Screen;
class QWindow;

/*
 * ScreenController monitors the Mir display configuration and compositor status, and updates
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

class ScreenController : public QObject
{
    Q_OBJECT
public:
    explicit ScreenController(QObject *parent = 0);

    Screen* getUnusedScreen();
    QList<Screen*> screens() const { return m_screenList; }
    bool compositing() const { return m_compositing; }

    QWindow* getWindowForPoint(const QPoint &point);

Q_SIGNALS:
    void screenAdded(Screen *screen);

public Q_SLOTS:
    void update();

public:
    // called by MirServer
    void init(const std::shared_ptr<mir::graphics::Display> &display,
              const std::shared_ptr<mir::compositor::Compositor> &compositor);
    void terminate();

    // override for testing purposes
    virtual Screen *createScreen(const mir::graphics::DisplayConfigurationOutput &output) const;

protected Q_SLOTS:
    void onCompositorStarting();
    void onCompositorStopping();

private:
    Screen* findScreenWithId(const QList<Screen*> &list, const mir::graphics::DisplayConfigurationOutputId id);

    std::weak_ptr<mir::graphics::Display> m_display;
    std::shared_ptr<mir::compositor::Compositor> m_compositor;
    QList<Screen*> m_screenList;
    bool m_compositing;
};

#endif // SCREENCONTROLLER_H
