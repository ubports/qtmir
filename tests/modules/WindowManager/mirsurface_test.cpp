/*
 * Copyright (C) 2014-2017 Canonical, Ltd.
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

#define MIR_INCLUDE_DEPRECATED_EVENT_HEADER

struct MirEvent {}; // otherwise won't compile otherwise due to incomplete type

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QLoggingCategory>
#include <QTest>
#include <QSignalSpy>

// src/common
#include "windowmodelnotifier.h"

// the test subject
#include <Unity/Application/mirsurface.h>

#include <Unity/Application/timer.h>
#include <Unity/Application/compositortextureprovider.h>

// mirtest
#include <mir/test/doubles/stub_session.h>
#include <mir/test/doubles/stub_surface.h>

// tests/framework
#include "stub_buffer.h"
#include "stub_windowcontroller.h"
#include "mock_renderable.h"

// tests/modules/common
#include <surfaceobserver.h>

// mir
#include <mir/scene/surface_creation_parameters.h>
#include <mir/version.h>

// miral
#include <miral/window.h>
#include <miral/window_info.h>

using namespace qtmir;

namespace ms = mir::scene;
using StubSurface = mir::test::doubles::StubSurface;
using StubSession = mir::test::doubles::StubSession;

using namespace testing;

struct MockSurface : public StubSurface
{
    MOCK_CONST_METHOD1(buffers_ready_for_compositor, int(void const*));
    MOCK_CONST_METHOD0(visible, bool());
    MOCK_CONST_METHOD0(state, MirWindowState());
    MOCK_CONST_METHOD1(generate_renderables,mir::graphics::RenderableList(mir::compositor::CompositorID id));
};

struct FakeCompositorTextureProvider: CompositorTextureProvider
{
    QSGTexture *createTexture() const override
    {
        return nullptr;
    }

};

class MirSurfaceTest : public ::testing::Test
{
public:
    MirSurfaceTest()
    {
        // We don't want the logging spam cluttering the test results
        QLoggingCategory::setFilterRules(QStringLiteral("qtmir.surfaces=false"));
    }

    const std::shared_ptr<StubSession> stubSession{std::make_shared<StubSession>()};
    const std::shared_ptr<StubSurface> stubSurface{std::make_shared<StubSurface>()};
};

TEST_F(MirSurfaceTest, UpdateTextureBeforeDraw)
{
    int argc = 0;
    char* argv[0];
    QCoreApplication qtApp(argc, argv); // app for deleteLater event

    auto mockSurface = std::make_shared<NiceMock<MockSurface>>();
    miral::Window mockWindow(stubSession, mockSurface);
    ms::SurfaceCreationParameters spec;
    miral::WindowInfo mockWindowInfo(mockWindow, spec);
    auto mockRenderable = std::make_shared<NiceMock<mir::graphics::MockRenderable>>();

    auto fakeTextureProvider = new FakeCompositorTextureProvider;
    // request multiple textures to simulate multiple screens.
    fakeTextureProvider->texture((qintptr)123);
    fakeTextureProvider->texture((qintptr)124);

    EXPECT_CALL(*mockSurface.get(),buffers_ready_for_compositor(_))
        .WillRepeatedly(Return(1));

    EXPECT_CALL(*mockSurface.get(),generate_renderables(_))
        .WillRepeatedly(Return(mir::graphics::RenderableList{mockRenderable}));

    EXPECT_CALL(*mockRenderable.get(),buffer())
        .WillRepeatedly(Return(std::make_shared<mir::graphics::StubBuffer>()));

    qtmir::MirSurface surface(mockWindowInfo, nullptr);
    surface.setTextureProvider(fakeTextureProvider);
#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 30, 0)
    surface.surfaceObserver()->frame_posted(NULL, 1, mir::geometry::Size{1,1});
#else
    surface.surfaceObserver()->frame_posted(1, mir::geometry::Size{1,1});
#endif

    QSignalSpy spyFrameDropped(&surface, SIGNAL(frameDropped()));
    QTest::qWait(300);
    ASSERT_TRUE(spyFrameDropped.count() > 0);
}

/*
 * Test that MirSurface.visible is recalculated after the client swaps the first frame.
 * A surface is not considered visible unless it has a non-hidden & non-minimized state, and
 * it has drawn at least one frame.
 */
TEST_F(MirSurfaceTest, EnsureVisiblePropertyRecalculatedAfterFrameSwap)
{
    auto mockSurface = std::make_shared<NiceMock<MockSurface>>();
    miral::Window mockWindow(stubSession, mockSurface);
    ms::SurfaceCreationParameters spec;
    miral::WindowInfo mockWindowInfo(mockWindow, spec);

    EXPECT_CALL(*mockSurface.get(),state())
        .WillRepeatedly(Return(mir_window_state_maximized));
    EXPECT_CALL(*mockSurface.get(),visible())
        .WillOnce(Return(false));

    qtmir::MirSurface surface(mockWindowInfo, nullptr);

    EXPECT_FALSE(surface.visible());

    EXPECT_CALL(*mockSurface.get(),visible())
        .WillOnce(Return(true));

    surface.setReady();
    EXPECT_TRUE(surface.visible());
}

/*
 * Test that a surface whose client fails to comply with a close request will eventually comply.
 */
struct MockWindowModelController : public StubWindowModelController
{
    MOCK_METHOD1(requestClose, void(const miral::Window &));
};

TEST_F(MirSurfaceTest, failedSurfaceCloseEventuallyDestroysSurface)
{
    int argc = 0;
    char* argv[0];
    QCoreApplication qtApp(argc, argv); // app for deleteLater event

    miral::Window mockWindow(stubSession, stubSurface);
    ms::SurfaceCreationParameters spec;
    miral::WindowInfo mockWindowInfo(mockWindow, spec);
    MockWindowModelController controller;

    qtmir::MirSurface surface(mockWindowInfo, &controller);

    bool surfaceDeleted = false;
    QObject::connect(&surface, &QObject::destroyed, &surface, [&surfaceDeleted](){ surfaceDeleted = true; });

    QSharedPointer<FakeTimeSource> fakeTimeSource(new FakeTimeSource);
    QPointer<FakeTimer> fakeTimer(new FakeTimer(fakeTimeSource));
    surface.setCloseTimer(fakeTimer.data()); // surface takes ownership of the timer

    qintptr view = (qintptr)1;
    surface.registerView(view);

    EXPECT_CALL(controller, requestClose(_))
        .Times(1);

    surface.close();

    if (fakeTimer->isRunning()) {
        // Simulate that closeTimer has timed out.
        fakeTimeSource->m_msecsSinceReference = fakeTimer->nextTimeoutTime() + 1;
        fakeTimer->update();
    }

    // clean up
    surface.setLive(false);
    surface.unregisterView(view);
}
