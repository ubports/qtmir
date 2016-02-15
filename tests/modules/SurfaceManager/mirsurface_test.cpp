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

#define MIR_INCLUDE_DEPRECATED_EVENT_HEADER

#include <gtest/gtest.h>

#include <QLoggingCategory>
#include <QTest>
#include <QSignalSpy>

// the test subject
#include <Unity/Application/mirsurface.h>

// tests/modules/common
#include <fake_session.h>
#include <mock_surface.h>
#include <surfaceobserver.h>

// mirserver
#include <sizehints.h>

using namespace qtmir;

namespace ms = mir::scene;

class MirSurfaceTest : public ::testing::Test
{
public:
    MirSurfaceTest()
    {
        // We don't want the logging spam cluttering the test results
        QLoggingCategory::setFilterRules(QStringLiteral("qtmir.surfaces=false"));
    }
};

TEST_F(MirSurfaceTest, UpdateTextureBeforeDraw)
{
    using namespace testing;

    int argc = 0;
    char* argv[0];
    QCoreApplication qtApp(argc, argv); // app for deleteLater event

    auto fakeSession = new FakeSession();
    auto mockSurface = std::make_shared<NiceMock<ms::MockSurface>>();
    auto surfaceObserver = std::make_shared<SurfaceObserver>();

    EXPECT_CALL(*mockSurface.get(),buffers_ready_for_compositor(_))
        .WillRepeatedly(Return(1));

    MirSurface *surface = new MirSurface(mockSurface, fakeSession, nullptr, surfaceObserver, SizeHints());
    surfaceObserver->frame_posted(1, mir::geometry::Size{1,1});

    QSignalSpy spyFrameDropped(surface, SIGNAL(frameDropped()));
    QTest::qWait(300);
    ASSERT_TRUE(spyFrameDropped.count() > 0);

    delete fakeSession;
    delete surface;
}


TEST_F(MirSurfaceTest, DeleteMirSurfaceOnLastNonLiveUnregisterView)
{
    using namespace testing;

    int argc = 0;
    char* argv[0];
    QCoreApplication qtApp(argc, argv); // app for deleteLater event

    auto fakeSession = new FakeSession();
    auto mockSurface = std::make_shared<NiceMock<ms::MockSurface>>();

    MirSurface *surface = new MirSurface(mockSurface, fakeSession, nullptr, nullptr, SizeHints());
    bool surfaceDeleted = false;
    QObject::connect(surface, &QObject::destroyed, surface, [&surfaceDeleted](){ surfaceDeleted = true; });

    qintptr view1 = (qintptr)1;
    qintptr view2 = (qintptr)2;

    surface->registerView(view1);
    surface->registerView(view2);
    surface->setLive(false);
    EXPECT_FALSE(surfaceDeleted);

    surface->unregisterView(view1);
    surface->unregisterView(view2);

    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    EXPECT_TRUE(surfaceDeleted);

    delete fakeSession;
}


TEST_F(MirSurfaceTest, DoNotDeleteMirSurfaceOnLastLiveUnregisterView)
{
    using namespace testing;

    int argc = 0;
    char* argv[0];
    QCoreApplication qtApp(argc, argv); // app for deleteLater event

    auto fakeSession = new FakeSession();
    auto mockSurface = std::make_shared<NiceMock<ms::MockSurface>>();

    MirSurface *surface = new MirSurface(mockSurface, fakeSession, nullptr, nullptr, SizeHints());
    bool surfaceDeleted = false;
    QObject::connect(surface, &QObject::destroyed, surface, [&surfaceDeleted](){ surfaceDeleted = true; });

    qintptr view1 = (qintptr)1;
    qintptr view2 = (qintptr)2;

    surface->registerView(view1);
    surface->registerView(view2);

    surface->unregisterView(view1);
    surface->unregisterView(view2);

    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    EXPECT_FALSE(surfaceDeleted);

    delete fakeSession;
    delete surface;
}
