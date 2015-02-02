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
        std::shared_ptr<mir::shell::InputTargeter> const& input_targeter,
        std::shared_ptr<mir::scene::SurfaceCoordinator> const& surface_coordinator,
        std::shared_ptr<mir::scene::SessionCoordinator> const& session_coordinator,
        std::shared_ptr<mir::scene::PromptSessionManager> const& prompt_session_manager,
        std::shared_ptr<mir::shell::DisplayLayout> const& display_layout);

    virtual mir::frontend::SurfaceId create_surface(std::shared_ptr<mir::scene::Session> const& session, mir::scene::SurfaceCreationParameters const& params);

    int set_surface_attribute(
        std::shared_ptr<mir::scene::Session> const& session,
        std::shared_ptr<mir::scene::Surface> const& surface,
        MirSurfaceAttrib attrib,
        int value) override;

Q_SIGNALS:
    void surfaceAttributeChanged(mir::scene::Surface const*, const MirSurfaceAttrib, const int);

private:
    std::shared_ptr<mir::shell::DisplayLayout> const m_displayLayout;
};

#endif /* QPAMIRSERVER_SHELL_H */
