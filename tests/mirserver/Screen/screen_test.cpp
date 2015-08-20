/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mir/graphics/display_configuration.h"
#include "fake_displayconfigurationoutput.h"

#include <screen.h>

using namespace ::testing;

namespace mg = mir::graphics;
namespace geom = mir::geometry;

class ScreenTest : public ::testing::Test {
protected:
    void SetUp() override;
};

void ScreenTest::SetUp()
{
    if (!qEnvironmentVariableIsSet("QT_ACCEL_FILEPATH")) {
        // Trick Qt >= 5.4.1 to load the generic sensors
        qputenv("QT_ACCEL_FILEPATH", "dummy");
    }

    Screen::skipDBusRegistration = true;
}

TEST_F(ScreenTest, OrientationSensor)
{
    Screen *screen = new Screen(fakeOutput1);

    // Default state should be active
    ASSERT_TRUE(screen->orientationSensorEnabled());

    screen->onDisplayPowerStateChanged(0,0);
    ASSERT_FALSE(screen->orientationSensorEnabled());

    screen->onDisplayPowerStateChanged(1,0);
    ASSERT_TRUE(screen->orientationSensorEnabled());
}

TEST_F(ScreenTest, ReadConfigurationFromDisplayConfig)
{
    Screen *screen = new Screen(fakeOutput1);

    EXPECT_EQ(screen->geometry(), QRect(0, 0, 150, 200));
    EXPECT_EQ(screen->availableGeometry(), QRect(0, 0, 150, 200));
    EXPECT_EQ(screen->depth(), 32);
    EXPECT_EQ(screen->format(), QImage::Format_RGBA8888);
    EXPECT_EQ(screen->refreshRate(), 59);
    EXPECT_EQ(screen->physicalSize(), QSize(1111, 2222));
    EXPECT_EQ(screen->outputType(), mg::DisplayConfigurationOutputType::dvid);
}

TEST_F(ScreenTest, ReadDifferentConfigurationFromDisplayConfig)
{
    Screen *screen = new Screen(fakeOutput2);

    EXPECT_EQ(screen->geometry(), QRect(500, 600, 1500, 2000));
    EXPECT_EQ(screen->availableGeometry(), QRect(500, 600, 1500, 2000));
    EXPECT_EQ(screen->depth(), 32);
    EXPECT_EQ(screen->format(), QImage::Format_RGBX8888);
    EXPECT_EQ(screen->refreshRate(), 75);
    EXPECT_EQ(screen->physicalSize(), QSize(1000, 2000));
    EXPECT_EQ(screen->outputType(), mg::DisplayConfigurationOutputType::lvds);
}
