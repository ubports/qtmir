/*
 * Copyright (C) 2015-2016 Canonical, Ltd.
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
#include "mock_gl_display_buffer.h"
#include "qtcompositor.h"
#include "fake_displayconfigurationoutput.h"

#include <mir/test/doubles/stub_display_buffer.h>

#include "testable_screensmodel.h"
#include "platformscreen.h"
#include "screenplatformwindow.h"

#include <QGuiApplication>
#include <QLoggingCategory>

using namespace ::testing;

namespace mg = mir::graphics;
namespace geom = mir::geometry;

using StubDisplayBuffer = mir::test::doubles::StubDisplayBuffer;

class ScreensModelTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;

    ScreensModel *screensModel;
    std::shared_ptr<StubDisplay> display;
    std::shared_ptr<StubDisplayListener> displayListener;
    std::shared_ptr<QtCompositor> compositor;
    QGuiApplication *app;
};

void ScreensModelTest::SetUp()
{
    setenv("QT_QPA_PLATFORM", "minimal", 1);
    PlatformScreen::skipDBusRegistration = true;

    // We don't want the logging spam cluttering the test results
    QLoggingCategory::setFilterRules(QStringLiteral("qtmir.*=false"));

    screensModel = new TestableScreensModel;
    display = std::make_shared<StubDisplay>();
    displayListener = std::make_shared<StubDisplayListener>();
    compositor = std::make_shared<QtCompositor>();

    static_cast<TestableScreensModel*>(screensModel)->do_init(display, compositor, displayListener);

    int argc = 0;
    char **argv = nullptr;
    setenv("QT_QPA_PLATFORM", "minimal", 1);
    app = new QGuiApplication(argc, argv);
}

void ScreensModelTest::TearDown()
{
    delete screensModel;
    delete app;
}

TEST_F(ScreensModelTest, SingleScreenFound)
{
    // Set up display state
    std::vector<mg::DisplayConfigurationOutput> config{fakeOutput1};
    std::vector<MockGLDisplayBuffer*> bufferConfig; // only used to match buffer with display, unecessary here
    display->setFakeConfiguration(config, bufferConfig);

    screensModel->update();

    ASSERT_EQ(1, screensModel->screens().count());
    PlatformScreen* screen = screensModel->screens().first();
    EXPECT_EQ(QRect(0, 0, 150, 200), screen->geometry());
}

TEST_F(ScreensModelTest, MultipleScreenFound)
{
    std::vector<mg::DisplayConfigurationOutput> config{fakeOutput1, fakeOutput2};
    std::vector<MockGLDisplayBuffer*> bufferConfig; // only used to match buffer with display, unecessary here
    display->setFakeConfiguration(config, bufferConfig);

    screensModel->update();

    ASSERT_EQ(2, screensModel->screens().count());
    EXPECT_EQ(QRect(0, 0, 150, 200), screensModel->screens().at(0)->geometry());
    EXPECT_EQ(QRect(500, 600, 1500, 2000), screensModel->screens().at(1)->geometry());
}

TEST_F(ScreensModelTest, ScreenAdded)
{
    std::vector<mg::DisplayConfigurationOutput> config{fakeOutput1};
    std::vector<MockGLDisplayBuffer*> bufferConfig; // only used to match buffer with display, unecessary here
    display->setFakeConfiguration(config, bufferConfig);

    screensModel->update();

    config.push_back(fakeOutput2);
    display->setFakeConfiguration(config, bufferConfig);

    ASSERT_EQ(1, screensModel->screens().count());
    EXPECT_EQ(QRect(0, 0, 150, 200), screensModel->screens().at(0)->geometry());

    screensModel->update();

    ASSERT_EQ(2, screensModel->screens().count());
    EXPECT_EQ(QRect(0, 0, 150, 200), screensModel->screens().at(0)->geometry());
    EXPECT_EQ(QRect(500, 600, 1500, 2000), screensModel->screens().at(1)->geometry());
}

TEST_F(ScreensModelTest, ScreenRemoved)
{
    std::vector<mg::DisplayConfigurationOutput> config{fakeOutput2, fakeOutput1};
    std::vector<MockGLDisplayBuffer*> bufferConfig; // only used to match buffer with display, unecessary here
    display->setFakeConfiguration(config, bufferConfig);

    screensModel->update();

    config.pop_back();
    display->setFakeConfiguration(config, bufferConfig);

    ASSERT_EQ(2, screensModel->screens().count());
    EXPECT_EQ(QRect(500, 600, 1500, 2000), screensModel->screens().at(0)->geometry());
    EXPECT_EQ(QRect(0, 0, 150, 200), screensModel->screens().at(1)->geometry());

    screensModel->update();

    ASSERT_EQ(1, screensModel->screens().count());
    EXPECT_EQ(QRect(500, 600, 1500, 2000), screensModel->screens().at(0)->geometry());
}

TEST_F(ScreensModelTest, MatchBufferWithDisplay)
{
    std::vector<mg::DisplayConfigurationOutput> config{fakeOutput1};
    MockGLDisplayBuffer buffer1;
    std::vector<MockGLDisplayBuffer*> buffers {&buffer1};

    geom::Rectangle buffer1Geom{{0, 0}, {150, 200}};
    EXPECT_CALL(buffer1, view_area())
            .WillRepeatedly(Return(buffer1Geom));

    display->setFakeConfiguration(config, buffers);
    screensModel->update();

    ASSERT_EQ(1, screensModel->screens().count());
    EXPECT_CALL(buffer1, make_current());
    static_cast<StubScreen*>(screensModel->screens().at(0))->makeCurrent();
}

TEST_F(ScreensModelTest, MultipleMatchBuffersWithDisplays)
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
    screensModel->update();

    ASSERT_EQ(2, screensModel->screens().count());
    EXPECT_CALL(buffer1, make_current());
    EXPECT_CALL(buffer2, make_current());
    static_cast<StubScreen*>(screensModel->screens().at(0))->makeCurrent();
    static_cast<StubScreen*>(screensModel->screens().at(1))->makeCurrent();
}
