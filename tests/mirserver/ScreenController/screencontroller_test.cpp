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

#include <gtest/gtest.h>
#include "gmock_fixes.h"

#include "stub_display.h"
#include "mock_main_loop.h"
#include "qtcompositor.h"
#include "fake_displayconfigurationoutput.h"

#include "testable_screencontroller.h"
#include "screen.h"
#include "screenwindow.h"

#include <QGuiApplication>

using namespace ::testing;

namespace mg = mir::graphics;
namespace geom = mir::geometry;

class ScreenControllerTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;

    ScreenController *screenController;
    std::shared_ptr<StubDisplay> display;
    std::shared_ptr<QtCompositor> compositor;
    QGuiApplication *app;
};

void ScreenControllerTest::SetUp()
{
    setenv("QT_QPA_PLATFORM", "minimal", 1);
    Screen::skipDBusRegistration = true;

    screenController = new TestableScreenController;
    display = std::make_shared<StubDisplay>();
    compositor = std::make_shared<QtCompositor>();

    static_cast<TestableScreenController*>(screenController)->do_init(display, compositor);

    int argc = 0;
    char **argv = nullptr;
    setenv("QT_QPA_PLATFORM", "minimal", 1);
    app = new QGuiApplication(argc, argv);
}

void ScreenControllerTest::TearDown()
{
    delete screenController;
}

TEST_F(ScreenControllerTest, SingleScreenFound)
{
    // Set up display state
    std::vector<mg::DisplayConfigurationOutput> config{fakeOutput1};
    std::vector<MockGLDisplayBuffer*> bufferConfig; // only used to match buffer with display, unecessary here
    display->setFakeConfiguration(config, bufferConfig);

    screenController->update();

    ASSERT_EQ(1, screenController->screens().count());
    Screen* screen = screenController->screens().first();
    EXPECT_EQ(QRect(0, 0, 150, 200), screen->geometry());
}

TEST_F(ScreenControllerTest, MultipleScreenFound)
{
    std::vector<mg::DisplayConfigurationOutput> config{fakeOutput1, fakeOutput2};
    std::vector<MockGLDisplayBuffer*> bufferConfig; // only used to match buffer with display, unecessary here
    display->setFakeConfiguration(config, bufferConfig);

    screenController->update();

    ASSERT_EQ(2, screenController->screens().count());
    EXPECT_EQ(QRect(0, 0, 150, 200), screenController->screens().at(0)->geometry());
    EXPECT_EQ(QRect(500, 600, 1500, 2000), screenController->screens().at(1)->geometry());
}

TEST_F(ScreenControllerTest, ScreenAdded)
{
    std::vector<mg::DisplayConfigurationOutput> config{fakeOutput1};
    std::vector<MockGLDisplayBuffer*> bufferConfig; // only used to match buffer with display, unecessary here
    display->setFakeConfiguration(config, bufferConfig);

    screenController->update();

    config.push_back(fakeOutput2);
    display->setFakeConfiguration(config, bufferConfig);

    ASSERT_EQ(1, screenController->screens().count());
    EXPECT_EQ(QRect(0, 0, 150, 200), screenController->screens().at(0)->geometry());

    screenController->update();

    ASSERT_EQ(2, screenController->screens().count());
    EXPECT_EQ(QRect(0, 0, 150, 200), screenController->screens().at(0)->geometry());
    EXPECT_EQ(QRect(500, 600, 1500, 2000), screenController->screens().at(1)->geometry());
}

TEST_F(ScreenControllerTest, ScreenRemoved)
{
    std::vector<mg::DisplayConfigurationOutput> config{fakeOutput2, fakeOutput1};
    std::vector<MockGLDisplayBuffer*> bufferConfig; // only used to match buffer with display, unecessary here
    display->setFakeConfiguration(config, bufferConfig);

    screenController->update();

    config.pop_back();
    display->setFakeConfiguration(config, bufferConfig);

    ASSERT_EQ(2, screenController->screens().count());
    EXPECT_EQ(QRect(500, 600, 1500, 2000), screenController->screens().at(0)->geometry());
    EXPECT_EQ(QRect(0, 0, 150, 200), screenController->screens().at(1)->geometry());

    screenController->update();

    ASSERT_EQ(1, screenController->screens().count());
    EXPECT_EQ(QRect(500, 600, 1500, 2000), screenController->screens().at(0)->geometry());
}

TEST_F(ScreenControllerTest, CheckPrioritizedGetUnusedScreen)
{
    std::vector<mg::DisplayConfigurationOutput> config{fakeOutput2, fakeOutput1};
    std::vector<MockGLDisplayBuffer*> bufferConfig; // only used to match buffer with display, unecessary here
    display->setFakeConfiguration(config, bufferConfig);

    screenController->update();

    auto screen = screenController->getUnusedScreen();
    EXPECT_EQ(mg::DisplayConfigurationOutputType::lvds, screen->outputType());
}

TEST_F(ScreenControllerTest, MatchBufferWithDisplay)
{
    std::vector<mg::DisplayConfigurationOutput> config{fakeOutput1};
    MockGLDisplayBuffer buffer1;
    std::vector<MockGLDisplayBuffer*> buffers {&buffer1};

    geom::Rectangle buffer1Geom{{0, 0}, {150, 200}};
    EXPECT_CALL(buffer1, view_area())
            .WillRepeatedly(Return(buffer1Geom));

    display->setFakeConfiguration(config, buffers);
    screenController->update();

    ASSERT_EQ(1, screenController->screens().count());
    EXPECT_CALL(buffer1, make_current());
    static_cast<StubScreen*>(screenController->screens().at(0))->makeCurrent();
}

TEST_F(ScreenControllerTest, MultipleMatchBuffersWithDisplays)
{
    std::vector<mg::DisplayConfigurationOutput> config{fakeOutput1, fakeOutput2};
    MockGLDisplayBuffer buffer1, buffer2;
    std::vector<MockGLDisplayBuffer*> buffers {&buffer1, &buffer2};

    geom::Rectangle buffer1Geom{{500, 600}, {1500, 2000}};
    geom::Rectangle buffer2Geom{{0, 0}, {150, 200}};
    EXPECT_CALL(buffer1, view_area())
            .WillRepeatedly(Return(buffer1Geom));
    EXPECT_CALL(buffer2, view_area())
            .WillRepeatedly(Return(buffer2Geom));

    display->setFakeConfiguration(config, buffers);
    screenController->update();

    ASSERT_EQ(2, screenController->screens().count());
    EXPECT_CALL(buffer1, make_current());
    EXPECT_CALL(buffer2, make_current());
    static_cast<StubScreen*>(screenController->screens().at(0))->makeCurrent();
    static_cast<StubScreen*>(screenController->screens().at(1))->makeCurrent();
}
