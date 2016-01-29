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

#ifndef MIR_TEST_DOUBLES_STUB_SCENE_SURFACE_H_
#define MIR_TEST_DOUBLES_STUB_SCENE_SURFACE_H_

#include "mir/scene/surface.h"
#include "stub_input_channel.h"

#include <memory>
#include <gmock/gmock.h>

namespace mir
{
namespace test
{
namespace doubles
{

class StubSceneSurface :
    public mir::scene::Surface
{
public:
    std::shared_ptr<StubInputChannel> channel;
    int fd;
    mir::input::InputReceptionMode input_mode{mir::input::InputReceptionMode::normal};

    StubSceneSurface(int fd)
        : channel(std::make_shared<StubInputChannel>(fd)), fd(fd)
    {
    }

    std::shared_ptr<mir::input::InputChannel> input_channel() const override
    {
        return channel;
    }

    mir::input::InputReceptionMode reception_mode() const override
    {
        return input_mode;
    }

    std::string name() const override { return {}; }
    geometry::Point top_left() const override { return {}; }
    geometry::Size client_size() const override { return {};}
    geometry::Size size() const override { return {}; }
    geometry::Rectangle input_bounds() const override { return {{},{}}; }
    bool input_area_contains(mir::geometry::Point const&) const override { return false; }

    graphics::RenderableList generate_renderables(compositor::CompositorID) const override { return {};}
    float alpha() const override { return 0.0f;}
    MirSurfaceType type() const override { return mir_surface_type_normal; }
    MirSurfaceState state() const override { return mir_surface_state_unknown; }

    void hide() override {}
    void show() override {}
    void move_to(geometry::Point const&) override {}
    void set_input_region(std::vector<geometry::Rectangle> const&) override {}
    void resize(geometry::Size const&) override {}
    void set_transformation(glm::mat4 const&) override {}
    void set_alpha(float) override {}
    void set_orientation(MirOrientation) override {}

    void add_observer(std::shared_ptr<scene::SurfaceObserver> const&) override {}
    void remove_observer(std::weak_ptr<scene::SurfaceObserver> const&) override {}

    void set_reception_mode(input::InputReceptionMode mode) override { input_mode = mode; }
    void consume(MirEvent const*) override {}

    void set_cursor_image(std::shared_ptr<graphics::CursorImage> const& /* image */) override {}
    std::shared_ptr<graphics::CursorImage> cursor_image() const override { return {}; }

    bool supports_input() const override { return true;}
    int client_input_fd() const override { return fd;}
    int configure(MirSurfaceAttrib, int) override { return 0; }
    int query(MirSurfaceAttrib) const override { return 0; }
};

}
}
}

#endif
