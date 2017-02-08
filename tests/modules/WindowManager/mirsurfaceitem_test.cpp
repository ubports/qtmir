/*
 * Copyright (C) 2014-2016 Canonical, Ltd.
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

#include <QLoggingCategory>
#include <QTest>
#include <private/qquickitem_p.h>

// the test subject
#include <Unity/Application/mirsurfaceitem.h>
// and friends
#include <Unity/Application/mirsurface.h>

// tests/framework
#include <fake_mirsurface.h>
#include <fake_session.h>
#include <mock_shell.h>
#include <fake_surface.h>

// tests/modules/common
#include <surfaceobserver.h>

using namespace qtmir;

class MirSurfaceItemTest : public ::testing::Test
{
public:
    MirSurfaceItemTest()
    {
        setenv("QT_QPA_PLATFORM", "minimal", 1);
        int argc = 0;
        char **argv = nullptr;
        m_app = new QGuiApplication(argc, argv);

        // We don't want the logging spam cluttering the test results
        QLoggingCategory::setFilterRules(QStringLiteral("qtmir.surfaces=false"));
    }
    virtual ~MirSurfaceItemTest()
    {
        delete m_app;
    }
    QGuiApplication *m_app;
};

/*
  Tests that even if Qt fails to finish a touch sequence, MirSurfaceItem will
  properly finish it when forwarding it to its mir::input::surface. So
  mir::input::surface will still consume a proper sequence of touch events
  (comprised of a begin, zero or more updates and an end).
 */
TEST_F(MirSurfaceItemTest, MissingTouchEnd)
{
    MirSurfaceItem *surfaceItem = new MirSurfaceItem;

    FakeMirSurface *fakeSurface = new FakeMirSurface;

    surfaceItem->setSurface(fakeSurface);
    surfaceItem->setConsumesInput(true);

    ulong timestamp = 1234;
    QList<QTouchEvent::TouchPoint> touchPoints;
    touchPoints.append(QTouchEvent::TouchPoint());

    touchPoints[0].setId(0);
    touchPoints[0].setState(Qt::TouchPointPressed);
    surfaceItem->processTouchEvent(QEvent::TouchBegin,
            timestamp, Qt::NoModifier, touchPoints, touchPoints[0].state());

    touchPoints[0].setState(Qt::TouchPointMoved);
    surfaceItem->processTouchEvent(QEvent::TouchUpdate,
            timestamp + 10, Qt::NoModifier, touchPoints, touchPoints[0].state());

    // Starting a new touch sequence (with touch 1) without ending the current one
    // (wich has touch 0).
    touchPoints[0].setId(1);
    touchPoints[0].setState(Qt::TouchPointPressed);
    surfaceItem->processTouchEvent(QEvent::TouchBegin,
            timestamp + 20, Qt::NoModifier, touchPoints, touchPoints[0].state());


    auto touchesReceived = fakeSurface->touchesReceived();

    // MirSurface should have received 4 events:
    // 1 - (id=0,down)
    // 2 - (id=0,move)
    // 3 - (id=0,up) <- that's the one MirSurfaceItem should have synthesized to keep the event stream sane.
    // 4 - (id=1,down)
    ASSERT_EQ(4, touchesReceived.count());

    ASSERT_EQ(0, touchesReceived[0].touchPoints[0].id());
    ASSERT_EQ(Qt::TouchPointPressed, touchesReceived[0].touchPoints[0].state());

    ASSERT_EQ(0, touchesReceived[1].touchPoints[0].id());
    ASSERT_EQ(Qt::TouchPointMoved, touchesReceived[1].touchPoints[0].state());

    ASSERT_EQ(0, touchesReceived[2].touchPoints[0].id());
    ASSERT_EQ(Qt::TouchPointReleased, touchesReceived[2].touchPoints[0].state());

    ASSERT_EQ(1, touchesReceived[3].touchPoints[0].id());
    ASSERT_EQ(Qt::TouchPointPressed, touchesReceived[3].touchPoints[0].state());

    delete surfaceItem;
    delete fakeSurface;
}

TEST_F(MirSurfaceItemTest, SetSurfaceInitializesVisiblity)
{
    MirSurfaceItem *surfaceItem = new MirSurfaceItem;
    surfaceItem->setVisible(false);

    FakeMirSurface *fakeSurface = new FakeMirSurface;
    surfaceItem->setSurface(fakeSurface);

    EXPECT_FALSE(fakeSurface->visible());

    delete surfaceItem;
    delete fakeSurface;
}

TEST_F(MirSurfaceItemTest, AggregateSurfaceVisibility)
{
    MirSurfaceItem *surfaceItem1 = new MirSurfaceItem;
    surfaceItem1->setVisible(true);
    MirSurfaceItem *surfaceItem2 = new MirSurfaceItem;
    surfaceItem1->setVisible(true);

    FakeMirSurface *fakeSurface = new FakeMirSurface;
    surfaceItem1->setSurface(fakeSurface);
    surfaceItem2->setSurface(fakeSurface);

    EXPECT_TRUE(fakeSurface->visible());

    surfaceItem1->setVisible(false);
    EXPECT_TRUE(fakeSurface->visible());

    surfaceItem2->setVisible(false);
    EXPECT_FALSE(fakeSurface->visible());

    surfaceItem1->setVisible(true);
    EXPECT_TRUE(fakeSurface->visible());

    delete surfaceItem1;
    EXPECT_FALSE(fakeSurface->visible());

    delete surfaceItem2;
    delete fakeSurface;
}

TEST_F(MirSurfaceItemTest, NoSurfaceActiveFocusIfItemDoesNotConsumeInput)
{
    using namespace testing;

    // All the stuff qtmir::MirSurface needs
    auto fakeSession = new FakeSession();
    std::shared_ptr<mir::scene::Surface> mockSurface = std::make_shared<mir::scene::FakeSurface>();
    auto surfaceObserver = std::make_shared<SurfaceObserver>();
    mir::shell::MockShell mockShell;

    MirSurface *surface = new MirSurface(mockSurface, fakeSession, &mockShell, surfaceObserver, CreationHints());

    MirSurfaceItem *surfaceItem = new MirSurfaceItem;
    QQuickItemPrivate *surfaceItemPrivate = QQuickItemPrivate::get(surfaceItem);

    surfaceItem->setConsumesInput(true);
    surfaceItemPrivate->activeFocus = true;

    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_focused))
        .Times(AtLeast(1));
    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_unfocused))
        .Times(0);

    surfaceItem->setSurface(surface);

    Mock::VerifyAndClearExpectations(&mockShell);
    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_focused))
        .Times(0);
    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_unfocused))
        .Times(AtLeast(1));

    surfaceItem->setConsumesInput(false);

    Mock::VerifyAndClearExpectations(&mockShell);
    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_focused))
        .Times(AtLeast(1));
    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_unfocused))
        .Times(0);

    surfaceItem->setConsumesInput(true);

    Mock::VerifyAndClearExpectations(&mockShell);
    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_focused))
        .Times(0);
    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_unfocused))
        .Times(AtLeast(1));

    delete surfaceItem;

    Mock::VerifyAndClearExpectations(&mockShell);

    // clean up
    delete surface;
    delete fakeSession;
}

TEST_F(MirSurfaceItemTest, AggregateSurfaceActiveFocus)
{
    using namespace testing;

    // All the stuff qtmir::MirSurface needs
    auto fakeSession = new FakeSession();
    std::shared_ptr<mir::scene::Surface> mockSurface = std::make_shared<mir::scene::FakeSurface>();
    auto surfaceObserver = std::make_shared<SurfaceObserver>();
    mir::shell::MockShell mockShell;

    MirSurface *surface = new MirSurface(mockSurface, fakeSession, &mockShell, surfaceObserver, CreationHints());

    MirSurfaceItem *surfaceItem1 = new MirSurfaceItem;
    QQuickItemPrivate *surfaceItem1Private = QQuickItemPrivate::get(surfaceItem1);

    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_focused))
        .Times(0);
    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_unfocused));

    surfaceItem1->setConsumesInput(true);
    surfaceItem1Private->activeFocus = false;
    surfaceItem1->setSurface(surface);

    Mock::VerifyAndClearExpectations(&mockShell);
    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_focused))
        .Times(AtLeast(1));
    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_unfocused))
        .Times(0);

    surfaceItem1Private->activeFocus = true;
    Q_EMIT surfaceItem1->activeFocusChanged(true);

    Mock::VerifyAndClearExpectations(&mockShell);
    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_focused))
        .Times(AtLeast(0)); // no harm in calling it unnecessarily
    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_unfocused))
        .Times(0);

    MirSurfaceItem *surfaceItem2 = new MirSurfaceItem;
    QQuickItemPrivate *surfaceItem2Private = QQuickItemPrivate::get(surfaceItem2);

    surfaceItem2->setConsumesInput(true);
    surfaceItem2Private->activeFocus = false;
    surfaceItem2->setSurface(surface);

    surfaceItem2Private->activeFocus = true;
    Q_EMIT surfaceItem2->activeFocusChanged(true);
    surfaceItem1Private->activeFocus = false;
    Q_EMIT surfaceItem1->activeFocusChanged(false);

    Mock::VerifyAndClearExpectations(&mockShell);
    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_focused))
        .Times(AtLeast(0)); // no harm in calling it unnecessarily
    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_unfocused))
        .Times(0);

    surfaceItem1Private->activeFocus = true;
    Q_EMIT surfaceItem1->activeFocusChanged(true);
    surfaceItem1->setConsumesInput(false);

    delete surfaceItem1;

    Mock::VerifyAndClearExpectations(&mockShell);
    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_focused))
        .Times(0);
    EXPECT_CALL(mockShell, set_surface_attribute(fakeSession->session(), mockSurface, mir_window_attrib_focus, (int)mir_window_focus_state_unfocused))
        .Times(AtLeast(1));

    delete surfaceItem2;

    Mock::VerifyAndClearExpectations(&mockShell);

    // clean up
    delete surface;
    delete fakeSession;
}
