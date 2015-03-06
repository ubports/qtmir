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

#ifndef QPAMIRSERVER_SHELL_H
#define QPAMIRSERVER_SHELL_H

#include <mir/shell/abstract_shell.h>
#include <QObject>

namespace mir {
    namespace shell {
        class DisplayLayout;
    }
}

class MirShell : public QObject, public mir::shell::AbstractShell
{
    Q_OBJECT

public:
    MirShell(
        const std::shared_ptr<mir::shell::InputTargeter> &inputTargeter,
        const std::shared_ptr<mir::scene::SurfaceCoordinator> &surfaceCoordinator,
        const std::shared_ptr<mir::scene::SessionCoordinator> &sessionCoordinator,
        const std::shared_ptr<mir::scene::PromptSessionManager> &promptSessionManager,
        const std::shared_ptr<mir::shell::DisplayLayout> &displayLayout);

    virtual mir::frontend::SurfaceId create_surface(const std::shared_ptr<mir::scene::Session>& session, const mir::scene::SurfaceCreationParameters &params);

    int set_surface_attribute(
        const std::shared_ptr<mir::scene::Session> &session,
        const std::shared_ptr<mir::scene::Surface> &surface,
        MirSurfaceAttrib attrib,
        int value) override;

Q_SIGNALS:
    void surfaceAttributeChanged(mir::scene::Surface const*, const MirSurfaceAttrib, const int);

private:
    std::shared_ptr<mir::shell::DisplayLayout> const m_displayLayout;
};

#endif /* QPAMIRSERVER_SHELL_H */
