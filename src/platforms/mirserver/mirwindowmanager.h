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

#ifndef QPAMIRSERVER_WINDOW_MANAGER_H
#define QPAMIRSERVER_WINDOW_MANAGER_H

#include <mir/shell/window_manager.h>

#include "sessionlistener.h"

#include <QObject>
#include <QSize>

namespace mir {
    namespace shell {
        class DisplayLayout;
        class FocusController;
    }
}

class MirWindowManager : public QObject, public mir::shell::WindowManager
{
    Q_OBJECT

public:
    static std::shared_ptr<MirWindowManager> create(
        const std::shared_ptr<mir::shell::DisplayLayout> &displayLayout,
        std::shared_ptr<::SessionListener> sessionListener);

Q_SIGNALS:
    // requires Qt::BlockingQueuedConnection!!
    void sessionAboutToCreateSurface(const std::shared_ptr<mir::scene::Session> &session,
                                     int type, QSize &size);
};

#endif /* QPAMIRSERVER_WINDOW_MANAGER_H */
