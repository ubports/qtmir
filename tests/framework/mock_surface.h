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

#ifndef MOCK_MIR_SCENE_SURFACE_H
#define MOCK_MIR_SCENE_SURFACE_H

#include <mir/scene/surface.h>
#include <gmock/gmock.h>

#include <string>
#include "mock_renderable.h"

namespace mir {
namespace scene {

struct MockSurface : public mir::scene::Surface
{
    MockSurface();
    virtual ~MockSurface();

    MOCK_CONST_METHOD0(name, std::string());
    MOCK_CONST_METHOD0(client_size, geometry::Size());
    MOCK_CONST_METHOD0(input_bounds, geometry::Rectangle());
    MOCK_CONST_METHOD0(top_left, geometry::Point());
    MOCK_CONST_METHOD0(size, geometry::Size());
    MOCK_CONST_METHOD1(generate_renderables,graphics::RenderableList(compositor::CompositorID id));
    MOCK_CONST_METHOD0(parent, std::shared_ptr<Surface>());


    MOCK_CONST_METHOD0(alpha, float());
    MOCK_CONST_METHOD0(type, MirSurfaceType());
    MOCK_CONST_METHOD0(state, MirSurfaceState());
    MOCK_METHOD0(hide, void());
    MOCK_METHOD0(show, void());
    MOCK_CONST_METHOD0(visible, bool());
    MOCK_METHOD1(move_to, void(geometry::Point const& top_left));
    MOCK_METHOD1(take_input_focus, void(std::shared_ptr<shell::InputTargeter> const& targeter));
    MOCK_METHOD1(set_input_region, void(std::vector<geometry::Rectangle> const& region));
    MOCK_METHOD1(allow_framedropping, void(bool));
    MOCK_METHOD1(resize, void(geometry::Size const& size));
    MOCK_METHOD1(set_transformation, void(glm::mat4 const& t));
    MOCK_METHOD1(set_alpha, void(float alpha));
    MOCK_METHOD1(set_orientation, void(MirOrientation orientation));
    MOCK_METHOD0(force_requests_to_complete, void());
    MOCK_METHOD1(set_cursor_image, void(std::shared_ptr<graphics::CursorImage> const& image));
    MOCK_CONST_METHOD0(cursor_image, std::shared_ptr<graphics::CursorImage>());
    MOCK_METHOD1(add_observer, void(std::shared_ptr<SurfaceObserver> const& observer));
    MOCK_METHOD1(remove_observer, void(std::weak_ptr<SurfaceObserver> const& observer));
    MOCK_CONST_METHOD0(input_channel, std::shared_ptr<input::InputChannel>());
    MOCK_METHOD1(set_reception_mode, void(input::InputReceptionMode mode));
    MOCK_METHOD0(request_client_surface_close, void());
    MOCK_CONST_METHOD1(buffers_ready_for_compositor, int(void const*));
    void set_keymap(xkb_rule_names const &) override;
    void rename(std::string const&) override;
    MOCK_METHOD1(set_streams, void(std::list<StreamInfo> const&));

    // from mir::input::surface
    MOCK_CONST_METHOD1(input_area_contains, bool(geometry::Point const& point));
    MOCK_CONST_METHOD0(reception_mode, input::InputReceptionMode());
    MOCK_METHOD1(consume, void(MirEvent const* event));
    void consume(MirEvent const& event) override;

    // from mir::frontend::surface
    MOCK_CONST_METHOD0(pixel_format, MirPixelFormat());
    MOCK_METHOD2(swap_buffers, void(graphics::Buffer* old_buffer, std::function<void(graphics::Buffer* new_buffer)> complete));
    MOCK_CONST_METHOD0(supports_input, bool());
    MOCK_CONST_METHOD0(client_input_fd, int());
    MOCK_METHOD2(configure, int(MirSurfaceAttrib attrib, int value));
    MOCK_CONST_METHOD1(query, int(MirSurfaceAttrib attrib));
    MOCK_CONST_METHOD0(primary_buffer_stream, std::shared_ptr<frontend::BufferStream>());

    // from mir::scene::SurfaceBufferAccess
    MOCK_METHOD1(with_most_recent_buffer_do, void(std::function<void(graphics::Buffer&)> const& exec));

    MOCK_METHOD2(set_cursor_stream, void(std::shared_ptr<frontend::BufferStream> const&, geometry::Displacement const&));
};

} // namespace scene
} // namespace mir

#endif // MOCK_MIR_SCENE_SURFACE_H
