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

//    std::weak_ptr<mir::scene::Session> focussed_application() const override;

    void set_focus_to(std::shared_ptr<mir::scene::Session> const& focus) override;

//    virtual std::shared_ptr<mir::scene::Session> open_session(
//        pid_t client_pid,
//        std::string const& name,
//        std::shared_ptr<mir::frontend::EventSink> const& sink);
//
//    virtual void close_session(std::shared_ptr<mir::scene::Session> const& session);

    void handle_surface_created(std::shared_ptr<mir::scene::Session> const& session) override;

//    virtual std::shared_ptr<mir::scene::PromptSession> start_prompt_session_for(
//        std::shared_ptr<mir::scene::Session> const& session,
//        mir::scene::PromptSessionCreationParameters const& params);
//
//    virtual void add_prompt_provider_for(
//        std::shared_ptr<mir::scene::PromptSession> const& prompt_session,
//        std::shared_ptr<mir::scene::Session> const& session);
//
//    virtual void stop_prompt_session(std::shared_ptr<mir::scene::PromptSession> const& prompt_session);

    virtual mir::frontend::SurfaceId create_surface(std::shared_ptr<mir::scene::Session> const& session, mir::scene::SurfaceCreationParameters const& params);

//    virtual void destroy_surface(std::shared_ptr<mir::scene::Session> const& session, mir::frontend::SurfaceId surface);
//
    virtual int set_surface_attribute(
        std::shared_ptr<mir::scene::Session> const& session,
        mir::frontend::SurfaceId surface_id,
        MirSurfaceAttrib attrib,
        int value);

//    virtual int get_surface_attribute(
//        std::shared_ptr<mir::scene::Session> const& session,
//        mir::frontend::SurfaceId surface_id,
//        MirSurfaceAttrib attrib);

Q_SIGNALS:
    void surfaceAttributeChanged(mir::scene::Surface const*, const MirSurfaceAttrib, const int);

private:
    std::shared_ptr<mir::shell::DisplayLayout> const m_displayLayout;
};

#endif /* QPAMIRSERVER_SHELL_H */
