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

#include "stub_session.h"

std::shared_ptr<mir::frontend::Surface> StubSession::get_surface(
    mir::frontend::SurfaceId /*surface*/) const
{
    return {};
}

std::string StubSession::name() const
{
    return {};
}

void StubSession::force_requests_to_complete()
{
}

pid_t StubSession::process_id() const
{
    return {0};
}

void StubSession::take_snapshot(
    mir::scene::SnapshotCallback const& /*snapshot_taken*/)
{
}

std::shared_ptr<mir::scene::Surface> StubSession::default_surface() const
{
    return {};
}

void StubSession::set_lifecycle_state(MirLifecycleState /*state*/)
{
}

void StubSession::send_display_config(
    mir::graphics::DisplayConfiguration const& /*configuration*/)
{
}

void StubSession::hide()
{
}

void StubSession::show()
{
}

void StubSession::start_prompt_session()
{
}

void StubSession::stop_prompt_session()
{
}

void StubSession::suspend_prompt_session()
{
}

void StubSession::resume_prompt_session()
{
}

mir::frontend::SurfaceId StubSession::create_surface(
    mir::scene::SurfaceCreationParameters const& /*params*/,
    std::shared_ptr<mir::frontend::EventSink> const& /*sink*/)
{
    return mir::frontend::SurfaceId{0};
}

void StubSession::destroy_surface(mir::frontend::SurfaceId /*surface*/)
{
}

void StubSession::destroy_surface(std::weak_ptr<mir::scene::Surface> const& /*surface*/)
{
}

std::shared_ptr<mir::scene::Surface> StubSession::surface(
    mir::frontend::SurfaceId /*surface*/) const
{
    return {};
}

std::shared_ptr<mir::scene::Surface> StubSession::surface_after(
    std::shared_ptr<mir::scene::Surface> const& /*ptr*/) const
{
    return {};
}

std::shared_ptr<mir::frontend::BufferStream> StubSession::get_buffer_stream(
    mir::frontend::BufferStreamId /*stream*/) const
{
    return {};
}

mir::frontend::BufferStreamId StubSession::create_buffer_stream(
    mir::graphics::BufferProperties const& /*props*/)
{
    return {};
}

void StubSession::destroy_buffer_stream(mir::frontend::BufferStreamId /*stream*/)
{
}

void StubSession::configure_streams(
    mir::scene::Surface& /*surface*/,
    std::vector<mir::shell::StreamSpecification> const& /*config*/)
{
}
