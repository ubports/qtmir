/*
 * Copyright © 2015 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QPAMIRSERVER_STUB_SURFACE_H
#define QPAMIRSERVER_STUB_SURFACE_H

#include <mir/scene/surface.h>

// mir::scene::Surface is a horribly wide interface to expose from Mir
// and there's not even a proper stub in the Mir-0.15 package mirtest-dev
struct StubSurface : mir::scene::Surface
{
    std::string name() const override;
    void move_to(mir::geometry::Point const& top_left) override;
    float alpha() const override;
    mir::geometry::Size size() const override;
    mir::geometry::Size client_size() const override;
    std::shared_ptr<mir::frontend::BufferStream> primary_buffer_stream() const override;
    void set_streams(std::list<mir::scene::StreamInfo> const& streams) override;
    bool supports_input() const override;
    int client_input_fd() const override;
    std::shared_ptr<mir::input::InputChannel> input_channel() const override;
    mir::input::InputReceptionMode reception_mode() const override;
    void set_reception_mode(mir::input::InputReceptionMode mode) override;
    void set_input_region(std::vector<mir::geometry::Rectangle> const& input_rectangles) override;
    void resize(mir::geometry::Size const& size) override;
    mir::geometry::Point top_left() const override;
    mir::geometry::Rectangle input_bounds() const override;
    bool input_area_contains(mir::geometry::Point const& point) const override;
    void consume(MirEvent const* event) override;
    void set_alpha(float alpha) override;
    void set_orientation(MirOrientation orientation) override;
    void set_transformation(glm::mat4 const&) override;
    bool visible() const override;
    mir::graphics::RenderableList generate_renderables(mir::compositor::CompositorID id) const override;
    int buffers_ready_for_compositor(void const* compositor_id) const override;
    MirSurfaceType type() const override;
    MirSurfaceState state() const override;
    int configure(MirSurfaceAttrib attrib, int value) override;
    int query(MirSurfaceAttrib attrib) const override;
    void hide() override;
    void show() override;
    void set_cursor_image(std::shared_ptr<mir::graphics::CursorImage> const& image) override;
    std::shared_ptr<mir::graphics::CursorImage> cursor_image() const override;
    void set_cursor_stream(std::shared_ptr<mir::frontend::BufferStream> const& stream, mir::geometry::Displacement const& hotspot) override;
    void request_client_surface_close() override;
    std::shared_ptr<Surface> parent() const override;
    void add_observer(std::shared_ptr<mir::scene::SurfaceObserver> const& observer) override;
    void remove_observer(std::weak_ptr<mir::scene::SurfaceObserver> const& observer) override;
    void set_keymap(xkb_rule_names const& rules) override;
    void rename(std::string const& title) override;
};

#endif //QPAMIRSERVER_STUB_SURFACE_H
