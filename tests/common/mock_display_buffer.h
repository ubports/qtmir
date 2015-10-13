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

#ifndef MOCK_DISPLAY_BUFFER_H
#define MOCK_DISPLAY_BUFFER_H

#include <mir/graphics/display_buffer.h>

#include <gmock/gmock.h>

class MockDisplayBuffer : public mir::graphics::DisplayBuffer
{
public:
    MockDisplayBuffer()
    {
        using namespace testing;
        ON_CALL(*this, view_area())
            .WillByDefault(Return(mir::geometry::Rectangle{{0,0},{0,0}}));
    }
    MOCK_CONST_METHOD0(view_area, mir::geometry::Rectangle());
    MOCK_METHOD0(make_current, void());
    MOCK_METHOD0(release_current, void());
    MOCK_METHOD0(gl_swap_buffers, void());
    MOCK_METHOD1(post_renderables_if_optimizable, bool(mir::graphics::RenderableList const&));
    MOCK_CONST_METHOD0(orientation, MirOrientation());
    MOCK_CONST_METHOD0(uses_alpha, bool());
};


#endif // MOCK_DISPLAY_BUFFER_H
