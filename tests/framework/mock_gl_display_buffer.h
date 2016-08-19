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

#ifndef MOCK_GL_DISPLAY_BUFFER_H
#define MOCK_GL_DISPLAY_BUFFER_H

#include <mir/test/doubles/null_display_buffer.h>
#include <mir/renderer/gl/render_target.h>

#include <gmock/gmock.h>

class MockGLDisplayBuffer : public mir::test::doubles::NullDisplayBuffer,
                            public mir::renderer::gl::RenderTarget
{
public:
    MOCK_CONST_METHOD0(view_area, mir::geometry::Rectangle());

    MOCK_METHOD0(make_current, void());
    MOCK_METHOD0(bind, void());
    MOCK_METHOD0(release_current, void());
    MOCK_METHOD0(swap_buffers, void());
};

#endif // MOCK_GL_DISPLAY_BUFFER_H
