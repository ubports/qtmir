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

#include "stub_surface.h"
#include <mir_toolkit/common.h>
//#include <mir/input/input_reception_mode.h>

std::string StubSurface::name() const
{
    return {};
}

void StubSurface::move_to(mir::geometry::Point const& /*top_left*/)
{
}

float StubSurface::alpha() const
{
    return 0;
}

mir::geometry::Size StubSurface::size() const
{
    return {};
}

mir::geometry::Size StubSurface::client_size() const
{
    return {};
}

std::shared_ptr <mir::frontend::BufferStream> StubSurface::primary_buffer_stream() const
{
    return {};
}

void StubSurface::set_streams(std::list<mir::scene::StreamInfo> const& /*streams*/)
{
}

bool StubSurface::supports_input() const
{
    return false;
}

int StubSurface::client_input_fd() const
{
    return 0;
}

std::shared_ptr <mir::input::InputChannel> StubSurface::input_channel() const
{
    return {};
}

mir::input::InputReceptionMode StubSurface::reception_mode() const
{
    return mir::input::InputReceptionMode::normal;
}

void StubSurface::set_reception_mode(mir::input::InputReceptionMode /*mode*/)
{
}

void StubSurface::set_input_region(std::vector<mir::geometry::Rectangle> const& /*input_rectangles*/)
{
}

void StubSurface::resize(mir::geometry::Size const& /*size*/)
{
}

mir::geometry::Point StubSurface::top_left() const
{
    return {};
}

mir::geometry::Rectangle StubSurface::input_bounds() const
{
    return {};
}

bool StubSurface::input_area_contains(mir::geometry::Point const& /*point*/) const
{
    return false;
}

void StubSurface::consume(MirEvent const* /*event*/)
{
}

void StubSurface::set_alpha(float /*alpha*/)
{
}

void StubSurface::set_orientation(MirOrientation /*orientation*/)
{
}

void StubSurface::set_transformation(glm::mat4 const& /*mat4*/)
{
}

bool StubSurface::visible() const
{
    return false;
}

mir::graphics::RenderableList StubSurface::generate_renderables(mir::compositor::CompositorID /*id*/) const
{
    return {};
}

int StubSurface::buffers_ready_for_compositor(void const* /*compositor_id*/) const
{
    return 0;
}

MirSurfaceType StubSurface::type() const
{
    return MirSurfaceType::mir_surface_type_normal;
}

MirSurfaceState StubSurface::state() const
{
    return MirSurfaceState::mir_surface_state_fullscreen;
}

int StubSurface::configure(MirSurfaceAttrib /*attrib*/, int value)
{
    return value;
}

int StubSurface::query(MirSurfaceAttrib /*attrib*/) const
{
    return 0;
}

void StubSurface::hide()
{
}

void StubSurface::show()
{
}

void StubSurface::set_cursor_image(std::shared_ptr<mir::graphics::CursorImage> const& /*image*/)
{
}

std::shared_ptr<mir::graphics::CursorImage> StubSurface::cursor_image() const
{
    return {};
}

void StubSurface::set_cursor_stream(
    std::shared_ptr<mir::frontend::BufferStream> const& /*stream*/,
    mir::geometry::Displacement const& /*hotspot*/)
{
}

void StubSurface::request_client_surface_close()
{
}

std::shared_ptr<mir::scene::Surface> StubSurface::parent() const
{
    return {};
}

void StubSurface::add_observer(std::shared_ptr<mir::scene::SurfaceObserver> const& /*observer*/)
{
}

void StubSurface::remove_observer(std::weak_ptr < mir::scene::SurfaceObserver > const& /*observer*/)
{
}

void StubSurface::set_keymap(MirInputDeviceId /*id*/, std::string const& /*model*/, std::string const& /*layout*/,
    std::string const& /*variant*/, std::string const& /*options*/)
{
}

void StubSurface::rename(std::string const& /*title*/)
{
}
