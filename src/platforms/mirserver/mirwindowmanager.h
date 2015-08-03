/*
 * Copyright © 2015 Canonical Ltd.
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

#include <QObject>

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

    static auto create(
        mir::shell::FocusController* focus_controller,
        const std::shared_ptr<mir::shell::DisplayLayout> &displayLayout)
        -> std::unique_ptr<MirWindowManager>;
};

#endif /* QPAMIRSERVER_WINDOW_MANAGER_H */
