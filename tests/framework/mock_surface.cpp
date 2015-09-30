#include "mock_surface.h"


mir::scene::MockSurface::MockSurface()
{
}

mir::scene::MockSurface::~MockSurface()
{
}

void mir::scene::MockSurface::rename(const std::string &) {}

void mir::scene::MockSurface::set_keymap(const xkb_rule_names &) {}

void mir::scene::MockSurface::consume(const MirEvent &event) { consume(&event); }
