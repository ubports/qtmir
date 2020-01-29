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
#include "mir/graphics/texture.h"

#include <stdexcept>

miral::GLBuffer::GLBuffer() = default;
miral::GLBuffer::~GLBuffer() = default;
miral::GLBuffer::GLBuffer(std::shared_ptr<mir::graphics::Buffer> const& buffer) :
    wrapped(buffer)
{
}

void miral::GLBuffer::reset(std::shared_ptr<mir::graphics::Buffer> const& buffer)
{
    wrapped = buffer;
}

miral::GLBuffer::operator bool() const
{
    return !!wrapped;
}

bool miral::GLBuffer::has_alpha_channel() const
{
    return wrapped &&
        (wrapped->pixel_format() == mir_pixel_format_abgr_8888
        || wrapped->pixel_format() == mir_pixel_format_argb_8888);
}

mir::geometry::Size miral::GLBuffer::size() const
{
    return wrapped->size();
}

void miral::GLBuffer::reset()
{
    wrapped.reset();
}

void miral::GLBuffer::bind()
{
    if (auto const texture = dynamic_cast<mir::graphics::gl::Texture*>(wrapped->native_buffer_base()))
    {
        texture->bind();
    }
    else
    {
        throw std::logic_error("Buffer does not support GL rendering");
    }
}
