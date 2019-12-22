/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
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

#ifndef MOCK_MIR_SCENE_SESSION_H
#define MOCK_MIR_SCENE_SESSION_H

#include <mir/scene/session.h>
#include <mir/graphics/display_configuration.h>
#include <mir/scene/surface_creation_parameters.h>
#include <mir/input/mir_input_config.h>
#include <mir/version.h>
#include <gmock/gmock.h>

#include <string>

namespace mir {
namespace scene {

struct MockSession : public Session
{
    MockSession();
    MockSession(std::string const& sessionName, pid_t processId);
    virtual ~MockSession();

    std::string name() const override;

    pid_t process_id() const override;

    MOCK_METHOD0(drop_outstanding_requests, void());

    MOCK_CONST_METHOD0(default_surface, std::shared_ptr<Surface>());
    MOCK_CONST_METHOD1(get_surface, std::shared_ptr<frontend::Surface>(frontend::SurfaceId));
    MOCK_CONST_METHOD1(surface, std::shared_ptr<scene::Surface>(frontend::SurfaceId));

    MOCK_METHOD1(take_snapshot, void(SnapshotCallback const&));
    MOCK_METHOD1(set_lifecycle_state, void(MirLifecycleState));
    MOCK_METHOD2(create_surface,
                 frontend::SurfaceId(SurfaceCreationParameters const&,
                                     std::shared_ptr<frontend::EventSink> const&));
    MOCK_METHOD1(destroy_surface, void (frontend::SurfaceId));
    MOCK_METHOD1(destroy_surface, void (std::weak_ptr<Surface> const& surface));

    MOCK_METHOD0(hide, void());
    MOCK_METHOD0(show, void());
    MOCK_METHOD1(send_display_config, void(graphics::DisplayConfiguration const&));
    MOCK_METHOD1(send_input_config, void(MirInputConfig const&));
    MOCK_METHOD3(configure_surface, int(frontend::SurfaceId, MirWindowAttrib, int));

    void start_prompt_session() override;
    void stop_prompt_session() override;
    void suspend_prompt_session() override;
    void resume_prompt_session() override;
    std::shared_ptr<Surface> surface_after(std::shared_ptr<Surface> const&) const override;

    MOCK_CONST_METHOD1(get_buffer_stream, std::shared_ptr<frontend::BufferStream>(frontend::BufferStreamId));
#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(1, 5, 0)
    MOCK_METHOD1(destroy_buffer_stream, void(std::shared_ptr<frontend::BufferStream> const&));
    MOCK_METHOD1(create_buffer_stream, std::shared_ptr<mir::compositor::BufferStream>(graphics::BufferProperties const&));
#else
    MOCK_METHOD1(destroy_buffer_stream, void(frontend::BufferStreamId));
    MOCK_METHOD1(create_buffer_stream, frontend::BufferStreamId(graphics::BufferProperties const&));
#endif
    void configure_streams(Surface&, std::vector<shell::StreamSpecification> const&) override;

    void send_error(ClientVisibleError const&) override;

#if MIR_SERVER_VERSION < MIR_VERSION_NUMBER(0, 28, 0)
#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 27, 0)
    graphics::BufferID create_buffer(geometry::Size, MirPixelFormat) override { return {}; }
    graphics::BufferID create_buffer(geometry::Size, uint32_t, uint32_t) override {return {}; }
#endif
#endif

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(1, 5, 0)
    auto create_surface(
#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(1, 6, 0)
        std::shared_ptr<Session> const&,
#endif
        SurfaceCreationParameters const&,
        std::shared_ptr<mir::scene::SurfaceObserver> const&) -> std::shared_ptr<Surface> override { return nullptr; };
    void destroy_surface(std::shared_ptr<Surface> const&) override {};
#endif
private:

    graphics::BufferID create_buffer(graphics::BufferProperties const&) { return graphics::BufferID{0}; }
    void destroy_buffer(graphics::BufferID) {}
    std::shared_ptr<graphics::Buffer> get_buffer(graphics::BufferID) { return {}; }

    std::string m_sessionName;
    pid_t m_sessionId;
};

} // namespace scene
} // namespace mir

#endif // MOCK_MIR_SCENE_SESSION_H
