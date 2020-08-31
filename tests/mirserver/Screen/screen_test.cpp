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
#include <orientationsensor.h>

#include <QSensorManager>

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
        // Tell Qt >= 5.7 to use the generic orientation sensor
        // since the proper linux one is not always running
        // in test environments making the test fail
        if (QSensorManager::isBackendRegistered("QOrientationSensor", "iio-sensor-proxy.orientationsensor")) {
            QSensorManager::unregisterBackend("QOrientationSensor", "iio-sensor-proxy.orientationsensor");
        }
    }

    OrientationSensor::skipDBusRegistration = true;
}

/*
TEST_F(ScreenTest, OrientationSensorForExternalDisplay)
{
    auto orientationSensor = std::make_shared<OrientationSensor>();
    orientationSensor->start();
    Screen *screen = new Screen(fakeOutput1, orientationSensor); // is external display (dvi)

    // Default state should be disabled
    ASSERT_FALSE(screen->orientationSensorEnabled());

    orientationSensor->onDisplayPowerStateChanged(0,0);
    ASSERT_FALSE(orientationSensor->enabled());

    orientationSensor->onDisplayPowerStateChanged(1,0);
    ASSERT_FALSE(orientationSensor->enabled());
}
*/

TEST_F(ScreenTest, OrientationSensorForInternalDisplay)
{
    auto orientationSensor = std::make_shared<OrientationSensor>();
    orientationSensor->start();
    Screen *screen = new Screen(fakeOutput2, orientationSensor); // is internal display

    // Default state should be active
    ASSERT_TRUE(screen->orientationSensorEnabled());

    orientationSensor->onDisplayPowerStateChanged(0,0);
    ASSERT_FALSE(orientationSensor->enabled());

    orientationSensor->onDisplayPowerStateChanged(1,0);
    ASSERT_TRUE(orientationSensor->enabled());
}

TEST_F(ScreenTest, ReadConfigurationFromDisplayConfig)
{
    Screen *screen = new Screen(fakeOutput1, std::make_shared<OrientationSensor>());

    EXPECT_EQ(screen->geometry(), QRect(0, 0, 150, 200));
    EXPECT_EQ(screen->availableGeometry(), QRect(0, 0, 150, 200));
    EXPECT_EQ(screen->depth(), 32);
    EXPECT_EQ(screen->format(), QImage::Format_RGBA8888);
    EXPECT_EQ(screen->refreshRate(), 59);
    EXPECT_EQ(screen->physicalSize(), QSize(1111, 2222));
    EXPECT_EQ(screen->outputType(), qtmir::OutputTypes::DVID);
}

TEST_F(ScreenTest, ReadDifferentConfigurationFromDisplayConfig)
{
    Screen *screen = new Screen(fakeOutput2, std::make_shared<OrientationSensor>());

    EXPECT_EQ(screen->geometry(), QRect(500, 600, 1500, 2000));
    EXPECT_EQ(screen->availableGeometry(), QRect(500, 600, 1500, 2000));
    EXPECT_EQ(screen->depth(), 32);
    EXPECT_EQ(screen->format(), QImage::Format_RGBX8888);
    EXPECT_EQ(screen->refreshRate(), 75);
    EXPECT_EQ(screen->physicalSize(), QSize(1000, 2000));
    EXPECT_EQ(screen->outputType(), qtmir::OutputTypes::LVDS);
}
