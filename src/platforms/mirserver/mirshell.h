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

#include <mir/shell/shell_wrapper.h>
#include <QObject>

namespace mir {
    namespace shell {
        class DisplayLayout;
    }
}

class MirShell : public QObject, public mir::shell::ShellWrapper
{
    Q_OBJECT

public:
    MirShell(
        std::shared_ptr<mir::shell::Shell> const& wrapped,
        std::shared_ptr<mir::shell::DisplayLayout> const& display_layout);

    void focus_next() override;

    void set_focus_to(std::shared_ptr<mir::scene::Session> const& focus) override;

    void handle_surface_created(std::shared_ptr<mir::scene::Session> const& session) override;

    virtual mir::frontend::SurfaceId create_surface(std::shared_ptr<mir::scene::Session> const& session, mir::scene::SurfaceCreationParameters const& params);

    virtual int set_surface_attribute(
        std::shared_ptr<mir::scene::Session> const& session,
        mir::frontend::SurfaceId surface_id,
        MirSurfaceAttrib attrib,
        int value);

Q_SIGNALS:
    void surfaceAttributeChanged(mir::scene::Surface const*, const MirSurfaceAttrib, const int);

private:
    std::shared_ptr<mir::shell::DisplayLayout> const m_displayLayout;
};

#endif /* QPAMIRSERVER_SHELL_H */
