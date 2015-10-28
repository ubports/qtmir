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

#ifndef QPAMIRSERVER_STUBSESSION_H
#define QPAMIRSERVER_STUBSESSION_H

#include <mir/scene/session.h>

struct StubSession : mir::scene::Session
{
    std::shared_ptr<mir::frontend::Surface> get_surface(mir::frontend::SurfaceId surface) const override;
    std::string name() const override;

    void force_requests_to_complete() override;
    pid_t process_id() const override;

    void take_snapshot(mir::scene::SnapshotCallback const& snapshot_taken) override;
    std::shared_ptr<mir::scene::Surface> default_surface() const override;
    void set_lifecycle_state(MirLifecycleState state) override;
    void send_display_config(mir::graphics::DisplayConfiguration const&) override;

    void hide() override;
    void show() override;

    void start_prompt_session() override;
    void stop_prompt_session() override;
    void suspend_prompt_session() override;
    void resume_prompt_session() override;

    mir::frontend::SurfaceId create_surface(
        mir::scene::SurfaceCreationParameters const& params,
        std::shared_ptr<frontend::EventSink> const& sink) override;

    void destroy_surface(mir::frontend::SurfaceId surface) override;

    std::shared_ptr<mir::scene::Surface> surface(mir::frontend::SurfaceId surface) const override;
    std::shared_ptr<mir::scene::Surface> surface_after(std::shared_ptr<mir::scene::Surface> const&) const override;

    std::shared_ptr<mir::frontend::BufferStream> get_buffer_stream(mir::frontend::BufferStreamId stream) const override;

    mir::frontend::BufferStreamId create_buffer_stream(mir::graphics::BufferProperties const& props) override;
    void destroy_buffer_stream(mir::frontend::BufferStreamId stream) override;
    void configure_streams(mir::scene::Surface& surface, std::vector<mir::shell::StreamSpecification> const& config) override;
};

#endif //QPAMIRSERVER_STUBSESSION_H
