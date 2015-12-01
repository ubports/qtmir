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

#ifndef MOCKDISPLAY_H
#define MOCKDISPLAY_H

#include <mir/graphics/display.h>
#include <mir/graphics/gl_context.h>

#include <gmock/gmock.h>
#include "gmock_fixes.h"

class MockDisplaySyncGroup : public mir::graphics::DisplaySyncGroup
{
public:
    MOCK_METHOD1(for_each_display_buffer, void(std::function<void(mir::graphics::DisplayBuffer&)> const& f));
    MOCK_METHOD0(post, void());
};

struct MockDisplay : public mir::graphics::Display
{
public:
    MOCK_METHOD1(for_each_display_sync_group, void(std::function<void(mir::graphics::DisplaySyncGroup&)> const&));
    MOCK_CONST_METHOD0(configuration, std::unique_ptr<mir::graphics::DisplayConfiguration>());
    MOCK_METHOD1(configure, void(mir::graphics::DisplayConfiguration const&));
    MOCK_METHOD2(register_configuration_change_handler,
                 void(mir::graphics::EventHandlerRegister&, mir::graphics::DisplayConfigurationChangeHandler const&));

    MOCK_METHOD3(register_pause_resume_handlers, void(mir::graphics::EventHandlerRegister&,
                                                      mir::graphics::DisplayPauseHandler const&,
                                                      mir::graphics::DisplayResumeHandler const&));
    MOCK_METHOD0(pause, void());
    MOCK_METHOD0(resume, void());
    MOCK_METHOD1(create_hardware_cursor, std::shared_ptr<mir::graphics::Cursor>(std::shared_ptr<mir::graphics::CursorImage> const&));
    MOCK_METHOD0(create_gl_context, std::unique_ptr<mir::graphics::GLContext>());
};



#endif // MOCKDISPLAY_H
