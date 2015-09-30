#include "stub_scene_surface.h"

mir::test::doubles::StubSceneSurface::StubSceneSurface(int fd)
    : channel(std::make_shared<StubInputChannel>(fd)), fd(fd)
{
}

mir::test::doubles::StubSceneSurface::~StubSceneSurface()
{
}

std::shared_ptr<mir::input::InputChannel> mir::test::doubles::StubSceneSurface::input_channel() const
{
    return channel;
}

mir::input::InputReceptionMode mir::test::doubles::StubSceneSurface::reception_mode() const
{
    return input_mode;
}

std::string mir::test::doubles::StubSceneSurface::name() const { return {}; }

mir::geometry::Point mir::test::doubles::StubSceneSurface::top_left() const { return {}; }

mir::geometry::Size mir::test::doubles::StubSceneSurface::client_size() const { return {};}

mir::geometry::Size mir::test::doubles::StubSceneSurface::size() const { return {}; }

mir::geometry::Rectangle mir::test::doubles::StubSceneSurface::input_bounds() const { return {{},{}}; }

bool mir::test::doubles::StubSceneSurface::input_area_contains(mir::geometry::Point const&) const { return false; }

mir::graphics::RenderableList mir::test::doubles::StubSceneSurface::generate_renderables(mir::compositor::CompositorID) const { return {};}

float mir::test::doubles::StubSceneSurface::alpha() const { return 0.0f; }

MirSurfaceType mir::test::doubles::StubSceneSurface::type() const { return mir_surface_type_normal; }

MirSurfaceState mir::test::doubles::StubSceneSurface::state() const { return mir_surface_state_unknown; }

void mir::test::doubles::StubSceneSurface::hide() {}

void mir::test::doubles::StubSceneSurface::show() {}

void mir::test::doubles::StubSceneSurface::move_to(const mir::geometry::Point &) {}

void mir::test::doubles::StubSceneSurface::set_input_region(const std::vector<mir::geometry::Rectangle> &) {}

void mir::test::doubles::StubSceneSurface::resize(const mir::geometry::Size &) {}

void mir::test::doubles::StubSceneSurface::set_transformation(const glm::mat4 &) {}

void mir::test::doubles::StubSceneSurface::set_alpha(float) {}

void mir::test::doubles::StubSceneSurface::set_orientation(MirOrientation) {}

void mir::test::doubles::StubSceneSurface::add_observer(const std::shared_ptr<mir::scene::SurfaceObserver> &) {}

void mir::test::doubles::StubSceneSurface::remove_observer(const std::weak_ptr<mir::scene::SurfaceObserver> &) {}

void mir::test::doubles::StubSceneSurface::set_reception_mode(mir::input::InputReceptionMode mode) { input_mode = mode; }

void mir::test::doubles::StubSceneSurface::consume(const MirEvent &) {}

void mir::test::doubles::StubSceneSurface::set_cursor_image(const std::shared_ptr<mir::graphics::CursorImage> &) {}

std::shared_ptr<mir::graphics::CursorImage> mir::test::doubles::StubSceneSurface::cursor_image() const { return {}; }

bool mir::test::doubles::StubSceneSurface::supports_input() const { return true; }

int mir::test::doubles::StubSceneSurface::client_input_fd() const { return fd;}

int mir::test::doubles::StubSceneSurface::configure(MirSurfaceAttrib, int) { return 0; }

int mir::test::doubles::StubSceneSurface::query(MirSurfaceAttrib) const { return 0; }
