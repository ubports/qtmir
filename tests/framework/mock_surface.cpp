#include "mock_surface.h"

namespace mir
{
namespace scene
{

MockSurface::MockSurface()
{
}

MockSurface::~MockSurface()
{
}

void MockSurface::rename(const std::string &) {}

void MockSurface::set_keymap(const xkb_rule_names &) {}

void MockSurface::consume(const MirEvent &event) { consume(&event); }

} // namespace scene
} // namespace mir
