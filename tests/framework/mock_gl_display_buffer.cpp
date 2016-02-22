/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include "mock_gl_display_buffer.h"

MockGLDisplayBuffer::MockGLDisplayBuffer()
{
    using namespace testing;
    ON_CALL(*this, view_area())
        .WillByDefault(Return(mir::geometry::Rectangle{{0,0},{0,0}}));
    ON_CALL(*this, native_display_buffer())
        .WillByDefault(Return(dynamic_cast<mir::graphics::NativeDisplayBuffer*>(this)));
}

MockGLDisplayBuffer::~MockGLDisplayBuffer()
{
}
