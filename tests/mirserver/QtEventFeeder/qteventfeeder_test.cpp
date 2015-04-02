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
 *
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <qteventfeeder.h>
#include <debughelpers.h>

#include <QGuiApplication>
#include <QWindow>

#include "mir/events/event_builders.h"

#include "mock_qtwindowsystem.h"

using ::testing::_;
using ::testing::AllOf;
using ::testing::AnyNumber;
using ::testing::Contains;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::Mock;
using ::testing::SizeIs;
using ::testing::Return;

// own gmock extensions
using ::testing::IsPressed;
using ::testing::IsReleased;
using ::testing::HasId;
using ::testing::StateIsMoved;

namespace mev = mir::events;

void PrintTo(const struct QWindowSystemInterface::TouchPoint& touchPoint, ::std::ostream* os) {
    *os << "TouchPoint("
        << "id=" << touchPoint.id
        << "," << touchPointStateToString(touchPoint.state)
        << ")";
}

class QtEventFeederTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;

    void setIrrelevantMockWindowSystemExpectations();

    MockQtWindowSystem *mockWindowSystem;
    QtEventFeeder *qtEventFeeder;
};

void QtEventFeederTest::SetUp()
{
    mockWindowSystem = new MockQtWindowSystem;

    EXPECT_CALL(*mockWindowSystem, registerTouchDevice(_));

    qtEventFeeder = new QtEventFeeder(mockWindowSystem);

    ASSERT_TRUE(Mock::VerifyAndClearExpectations(mockWindowSystem));
}

void QtEventFeederTest::TearDown()
{
    // mockWindowSystem will be deleted by QtEventFeeder
    delete qtEventFeeder;
}

void QtEventFeederTest::setIrrelevantMockWindowSystemExpectations()
{
    EXPECT_CALL(*mockWindowSystem, hasTargetWindow())
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mockWindowSystem, targetWindowGeometry())
        .Times(AnyNumber())
        .WillRepeatedly(Return(QRect(0,0,100,100)));
}


/*
   Mir sends a MirEvent([touch(id=0,state=pressed)]. QtEventFeeder happily forwards that to Qt.

   Then, Mir sends a MirEvent([touch(id=1,state=pressed)]). In MirEvents, every single active touch
   point must be listed in the event even if it didn't change at all in the meantime. So that's a bug.
   But instead of crashing or forwarding the bogus event stream down to Qt, QtEventFeeder should attempt
   to fix the situation by synthesizing a touch[id=1,state=released] to be send along with the
   touch(id=1,state=pressed) it got. So that Qt receives a correct touch event stream.
 */
TEST_F(QtEventFeederTest, GenerateMissingTouchEnd)
{

    setIrrelevantMockWindowSystemExpectations();

    EXPECT_CALL(*mockWindowSystem, handleTouchEvent(_,_,AllOf(SizeIs(1),
                                                              Contains(AllOf(HasId(0),
                                                                             IsPressed()))),_)).Times(1);

    auto ev1 = mev::make_event(MirInputDeviceId(), 123*1000000, 0);
    mev::add_touch(*ev1, /* touch ID */ 0, mir_touch_action_down, mir_touch_tooltype_unknown,
                   10, 10, 10, 1, 1, 10);
    qtEventFeeder->dispatch(*ev1);

    ASSERT_TRUE(Mock::VerifyAndClearExpectations(mockWindowSystem));

    setIrrelevantMockWindowSystemExpectations();

    EXPECT_CALL(*mockWindowSystem, handleTouchEvent(_,_,AllOf(SizeIs(2),
                                                              Contains(AllOf(HasId(0),IsReleased())),
                                                              Contains(AllOf(HasId(1),IsPressed()))
                                                             ),_)).Times(1);

    auto ev2 = mev::make_event(MirInputDeviceId(), 125*1000000, 0);
    mev::add_touch(*ev2, 1 /* touch ID */, mir_touch_action_down, mir_touch_tooltype_unknown,
                   10, 10, 10, 1, 1, 10);
    qtEventFeeder->dispatch(*ev2);

    ASSERT_TRUE(Mock::VerifyAndClearExpectations(mockWindowSystem));
}

TEST_F(QtEventFeederTest, PressSameTouchTwice)
{
    setIrrelevantMockWindowSystemExpectations();

    EXPECT_CALL(*mockWindowSystem, handleTouchEvent(_,_,AllOf(SizeIs(1),
                                                              Contains(AllOf(HasId(0),
                                                                             IsPressed()))),_)).Times(1);

    auto ev1 = mev::make_event(MirInputDeviceId(), 123*1000000, 0);
    mev::add_touch(*ev1, /* touch ID */ 0, mir_touch_action_down, mir_touch_tooltype_unknown,
                   10, 10, 10, 1, 1, 10);
    qtEventFeeder->dispatch(*ev1);

    ASSERT_TRUE(Mock::VerifyAndClearExpectations(mockWindowSystem));

    setIrrelevantMockWindowSystemExpectations();

    EXPECT_CALL(*mockWindowSystem, handleTouchEvent(_,_,AllOf(SizeIs(1),
                                                              Contains(AllOf(HasId(0), StateIsMoved()))
                                                             ),_)).Times(1);

    auto ev2 = mev::make_event(MirInputDeviceId(), 123*1000000, 0);
    mev::add_touch(*ev2, /* touch ID */ 0, mir_touch_action_down, mir_touch_tooltype_unknown,
                   10, 10, 10, 1, 1, 10);
    qtEventFeeder->dispatch(*ev2);

    ASSERT_TRUE(Mock::VerifyAndClearExpectations(mockWindowSystem));
}

