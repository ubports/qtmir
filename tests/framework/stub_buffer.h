/*
 * Copyright (C) 2017 Canonical, Ltd.
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

#ifndef STUB_MIR_GRAPHICS_BUFFER_H
#define STUB_MIR_GRAPHICS_BUFFER_H

#include <mir/graphics/buffer.h>

namespace mir {
namespace graphics {

class StubBuffer : public Buffer
{
public:
    StubBuffer() {}

    std::shared_ptr<NativeBuffer> native_buffer_handle() const override { return std::shared_ptr<NativeBuffer>(); }
    BufferID id() const override { return BufferID(); }
    geometry::Size size() const override { return geometry::Size(); }
    MirPixelFormat pixel_format() const override { return mir_pixel_format_invalid; }

    NativeBufferBase* native_buffer_base() override { return nullptr; }
};

} // namespace graphics
} // namespace mir

#endif // STUB_MIR_GRAPHICS_BUFFER_H
