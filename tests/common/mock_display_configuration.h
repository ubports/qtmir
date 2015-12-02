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

#ifndef MOCK_DISPLAY_CONFIGURATION_H
#define MOCK_DISPLAY_CONFIGURATION_H

#include <mir/graphics/display_configuration.h>

#include <gmock/gmock.h>
#include "gmock_fixes.h"

class MockDisplayConfiguration : public mir::graphics::DisplayConfiguration
{
public:
    MOCK_CONST_METHOD1(for_each_card, void(std::function<void(mir::graphics::DisplayConfigurationCard const&)>));

    MOCK_CONST_METHOD1(for_each_output, void(std::function<void(mir::graphics::DisplayConfigurationOutput const&)>));
    MOCK_METHOD1(for_each_output, void(std::function<void(mir::graphics::UserDisplayConfigurationOutput&)>));

    MOCK_CONST_METHOD0(valid, bool());
    MOCK_CONST_METHOD0(clone, std::unique_ptr<mir::graphics::DisplayConfiguration>());
};
#endif // MOCK_DISPLAY_CONFIGURATION_H
