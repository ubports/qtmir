/*
 * Copyright (C) 2020 UBports Foundation
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

#pragma once

// Provides the qt-windowmanager Wayland extension, which provides support for
// URL handling. We use url-dispatcher for the purpose.

#include "services.h"

#include <miral/wayland_extensions.h>

namespace qtmir
{
// The casing is deliberate, given 'Windowmanager' is all one word according to
// the Wayland spec
auto qtWindowmanagerExtension() -> miral::WaylandExtensions::Builder;
}
