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

#include "graphics_buffer.h"

#include <mir/graphics/buffer.h>
#include <mir/renderer/gl/texture_source.h>

#include <stdexcept>

mir::geometry::Size qtmir::size(mir::graphics::Buffer const &buffer)
{
    return buffer.size();
}

bool qtmir::hasAlphaChannel(mir::graphics::Buffer const &buffer)
{
        return buffer.pixel_format() == mir_pixel_format_abgr_8888
            || buffer.pixel_format() == mir_pixel_format_argb_8888;
}

void qtmir::glBindToTexture(mir::graphics::Buffer& m_mirBuffer)
{
    namespace mrg = mir::renderer::gl;

    auto const texture_source =
        dynamic_cast<mrg::TextureSource*>(m_mirBuffer.native_buffer_base());
    if (!texture_source)
        throw std::logic_error("Buffer does not support GL rendering");

    texture_source->gl_bind_to_texture();
}