/*
 * Copyright © 2017 Canonical Ltd.
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
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef QTMIR_GRAPHICS_BUFFER_H
#define QTMIR_GRAPHICS_BUFFER_H

#include <mir/geometry/size.h>

namespace mir { namespace graphics { class Buffer; }}

namespace qtmir
{
bool hasAlphaChannel(mir::graphics::Buffer const &buffer);
mir::geometry::Size size(mir::graphics::Buffer const &buffer);
void glBindToTexture(mir::graphics::Buffer& buffer);
}

#endif //QTMIR_GRAPHICS_BUFFER_H
