/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include "stub_scene_surface.h"

namespace mir
{
namespace test
{
namespace doubles
{

StubSceneSurface::StubSceneSurface(int fd)
    : channel(std::make_shared<StubInputChannel>(fd)), fd(fd)
{
}

StubSceneSurface::~StubSceneSurface()
{
}

std::shared_ptr<mir::input::InputChannel> StubSceneSurface::input_channel() const
{
    return channel;
}

mir::input::InputReceptionMode StubSceneSurface::reception_mode() const
{
    return input_mode;
}

std::string StubSceneSurface::name() const { return {}; }

mir::geometry::Point StubSceneSurface::top_left() const { return {}; }

mir::geometry::Size StubSceneSurface::client_size() const { return {};}

mir::geometry::Size StubSceneSurface::size() const { return {}; }

mir::geometry::Rectangle StubSceneSurface::input_bounds() const { return {{},{}}; }

bool StubSceneSurface::input_area_contains(mir::geometry::Point const&) const { return false; }

mir::graphics::RenderableList StubSceneSurface::generate_renderables(mir::compositor::CompositorID) const { return {};}

float StubSceneSurface::alpha() const { return 0.0f; }

MirSurfaceType StubSceneSurface::type() const { return mir_surface_type_normal; }

MirSurfaceState StubSceneSurface::state() const { return mir_surface_state_unknown; }

void StubSceneSurface::hide() {}

void StubSceneSurface::show() {}

void StubSceneSurface::move_to(const mir::geometry::Point &) {}

void StubSceneSurface::set_input_region(const std::vector<mir::geometry::Rectangle> &) {}

void StubSceneSurface::resize(const mir::geometry::Size &) {}

void StubSceneSurface::set_transformation(const glm::mat4 &) {}

void StubSceneSurface::set_alpha(float) {}

void StubSceneSurface::set_orientation(MirOrientation) {}

void StubSceneSurface::add_observer(const std::shared_ptr<mir::scene::SurfaceObserver> &) {}

void StubSceneSurface::remove_observer(const std::weak_ptr<mir::scene::SurfaceObserver> &) {}

void StubSceneSurface::set_reception_mode(mir::input::InputReceptionMode mode) { input_mode = mode; }

void StubSceneSurface::consume(const MirEvent *) {}

void StubSceneSurface::set_cursor_image(const std::shared_ptr<mir::graphics::CursorImage> &) {}

std::shared_ptr<mir::graphics::CursorImage> StubSceneSurface::cursor_image() const { return {}; }

bool StubSceneSurface::supports_input() const { return true; }

int StubSceneSurface::client_input_fd() const { return fd;}

int StubSceneSurface::configure(MirSurfaceAttrib, int) { return 0; }

int StubSceneSurface::query(MirSurfaceAttrib) const { return 0; }

} // namespace doubles
} // namespace test
} // namespace mir
