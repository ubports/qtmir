/*
 * Copyright (C) 2016 Canonical, Ltd.
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

#ifndef MOCK_MIR_SHELL_SHELL_H
#define MOCK_MIR_SHELL_SHELL_H

#include <mir/shell/shell.h>

#include <mir/scene/prompt_session_creation_parameters.h>
#include <mir/scene/surface_creation_parameters.h>
#include <mir/shell/surface_specification.h>

#include <gmock/gmock.h>

namespace mir {
namespace shell {

class MockShell : public Shell
{
public:

    // From mir::shell::Shell

    MOCK_METHOD3(open_session, std::shared_ptr<scene::Session>(
        pid_t client_pid,
        std::string const& name,
        std::shared_ptr<frontend::EventSink> const& sink));

    MOCK_METHOD1(close_session, void(std::shared_ptr<scene::Session> const& session));

    MOCK_METHOD2(start_prompt_session_for, std::shared_ptr<scene::PromptSession>(
        std::shared_ptr<scene::Session> const& session,
        scene::PromptSessionCreationParameters const& params));

    MOCK_METHOD2(add_prompt_provider_for, void (
        std::shared_ptr<scene::PromptSession> const& prompt_session,
        std::shared_ptr<scene::Session> const& session));

    MOCK_METHOD1(stop_prompt_session, void (std::shared_ptr<scene::PromptSession> const& prompt_session));

    MOCK_METHOD3(create_surface, frontend::SurfaceId (
        std::shared_ptr<scene::Session> const& session,
        scene::SurfaceCreationParameters const& params,
        std::shared_ptr<frontend::EventSink> const& sink));

    MOCK_METHOD3(modify_surface, void (
        std::shared_ptr<scene::Session> const& session,
        std::shared_ptr<scene::Surface> const& surface,
        shell::SurfaceSpecification  const& modifications));

    MOCK_METHOD2(destroy_surface, void (std::shared_ptr<scene::Session> const& session, frontend::SurfaceId surface));

    MOCK_METHOD4(set_surface_attribute, int (
        std::shared_ptr<scene::Session> const& session,
        std::shared_ptr<scene::Surface> const& surface,
        MirWindowAttrib attrib,
        int value));

    MOCK_METHOD2(get_surface_attribute, int (
        std::shared_ptr<scene::Surface> const& surface,
        MirWindowAttrib attrib));

    MOCK_METHOD3(raise_surface, void (
        std::shared_ptr<scene::Session> const& session,
        std::shared_ptr<scene::Surface> const& surface,
        uint64_t timestamp));

    // From mir::shell::FocusController

    MOCK_METHOD0(focus_next_session, void ());

    MOCK_CONST_METHOD0(focused_session, std::shared_ptr<scene::Session>());

    MOCK_METHOD2(set_focus_to, void (
        std::shared_ptr<scene::Session> const& focus_session,
        std::shared_ptr<scene::Surface> const& focus_surface));

    MOCK_CONST_METHOD0(focused_surface, std::shared_ptr<scene::Surface>());

    MOCK_CONST_METHOD1(surface_at, std::shared_ptr<scene::Surface> (geometry::Point cursor));

    MOCK_METHOD1(raise, void (SurfaceSet const& surfaces));

    // From mir::input::EventFilter

    MOCK_METHOD1(handle, bool (MirEvent const& event));

    // From mir::compositor::DisplayListener

    MOCK_METHOD1(add_display, void (geometry::Rectangle const& area));

    MOCK_METHOD1(remove_display, void (geometry::Rectangle const& area));
};

} // namespace shell
} // namespace mir

#endif // MOCK_MIR_SHELL_SHELL_H
