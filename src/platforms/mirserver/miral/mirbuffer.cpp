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
 */

#include "mirbuffer.h"

#include <mir/graphics/buffer.h>
#include <mir/renderer/gl/texture_source.h>

#include <stdexcept>

miral::GLBuffer::GLBuffer() = default;
miral::GLBuffer::~GLBuffer() = default;
miral::GLBuffer::GLBuffer(std::shared_ptr<mir::graphics::Buffer> const& buffer) :
    m_mirBuffer(buffer)
{
}

miral::GLBuffer& miral::GLBuffer::operator=(std::shared_ptr<mir::graphics::Buffer> const& buffer)
{
    m_mirBuffer = buffer;
    return *this;
}

bool miral::GLBuffer::has_buffer() const
{
    return !!m_mirBuffer;
}

bool miral::GLBuffer::has_alpha_channel() const
{
    return has_buffer() &&
        (m_mirBuffer->pixel_format() == mir_pixel_format_abgr_8888
        || m_mirBuffer->pixel_format() == mir_pixel_format_argb_8888);
}

mir::geometry::Size miral::GLBuffer::size() const
{
    return m_mirBuffer->size();
}

void miral::GLBuffer::reset()
{
    m_mirBuffer.reset();
}

void miral::GLBuffer::bind_to_texture()
{
    namespace mrg = mir::renderer::gl;

    auto const texture_source =
        dynamic_cast<mrg::TextureSource*>(m_mirBuffer->native_buffer_base());
    if (!texture_source)
        throw std::logic_error("Buffer does not support GL rendering");

    texture_source->gl_bind_to_texture();
}
