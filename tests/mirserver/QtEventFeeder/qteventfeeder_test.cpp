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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <qteventfeeder.h>
#include <debughelpers.h>

#include <QGuiApplication>
#include <QWindow>

#include "mir/events/event_builders.h"

#include "mock_qtwindowsystem.h"

#include <linux/input.h>
#include <xkbcommon/xkbcommon-keysyms.h>

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
using ::testing::IsStationary;
using ::testing::IsReleased;
using ::testing::HasId;
using ::testing::StateIsMoved;

namespace mev = mir::events;

// used by google mock in error messages
void PrintTo(const struct QWindowSystemInterface::TouchPoint& touchPoint, ::std::ostream* os) {
    *os << "TouchPoint("
        << "id=" << touchPoint.id
        << "," << touchPointStateToString(touchPoint.state)
        << ")";
}

::std::ostream& operator<<(::std::ostream& os, QTouchDevice*) {
    // actually don't care about its contents. Just to avoit having a raw
    // pointer address being printed in google mock error messages.
    return os << "QTouchDevice*";
}

class QtEventFeederTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;

    void setIrrelevantMockWindowSystemExpectations();

    MockQtWindowSystem *mockWindowSystem;
    QtEventFeeder *qtEventFeeder;
    QWindow *window;
    QGuiApplication *app;
};

void QtEventFeederTest::SetUp()
{
    mockWindowSystem = new MockQtWindowSystem;

    ASSERT_TRUE(mockWindowSystem->m_devices.count() == 0);

    qtEventFeeder = new QtEventFeeder(mockWindowSystem);

    ASSERT_TRUE(mockWindowSystem->m_devices.count() == 1);

    int argc = 0;
    char **argv = nullptr;
    setenv("QT_QPA_PLATFORM", "minimal", 1);
    app = new QGuiApplication(argc, argv);
    window = new QWindow;
}

void QtEventFeederTest::TearDown()
{
    // mockWindowSystem will be deleted by QtEventFeeder
    delete qtEventFeeder;
    delete window;
    delete app;
}

void QtEventFeederTest::setIrrelevantMockWindowSystemExpectations()
{
    EXPECT_CALL(*mockWindowSystem, getWindowForTouchPoint(_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(window));
    EXPECT_CALL(*mockWindowSystem, focusedWindow())
        .Times(AnyNumber())
        .WillRepeatedly(Return(window));
}


/*
   Mir sends a MirEvent([touch(id=0,state=pressed)]. QtEventFeeder happily forwards that to Qt.

   Then, Mir sends a MirEvent([touch(id=1,state=pressed)]). In MirEvents, every single active touch
   point must be listed in the event even if it didn't change at all in the meantime. So that's a bug.
   But instead of crashing or forwarding the bogus event stream down to Qt, QtEventFeeder should attempt
   to fix the situation by synthesizing a touch[id=0,state=released] to be send along with the
   touch(id=1,state=pressed) it got. So that Qt receives a correct touch event stream.
 */
TEST_F(QtEventFeederTest, GenerateMissingTouchEnd)
{

    setIrrelevantMockWindowSystemExpectations();

    EXPECT_CALL(*mockWindowSystem, handleTouchEvent(_,_,_,AllOf(SizeIs(1),
                                                              Contains(AllOf(HasId(0),
                                                                             IsPressed()))),_)).Times(1);

    auto ev1 = mev::make_event(MirInputDeviceId(), std::chrono::milliseconds(123), std::vector<uint8_t>{} /* cookie */, 0);
    mev::add_touch(*ev1, 0 /* touch ID */, mir_touch_action_down, mir_touch_tooltype_unknown,
                   10, 10, 10 /* x, y, pressure */,
                   1, 1, 10 /* touch major, minor, size */);
    qtEventFeeder->dispatch(*ev1);

    ASSERT_TRUE(Mock::VerifyAndClearExpectations(mockWindowSystem));

    setIrrelevantMockWindowSystemExpectations();

    // There can be only one pressed or released touch per event
    {
        InSequence sequence;

        EXPECT_CALL(*mockWindowSystem,
                    handleTouchEvent(_,_,_,AllOf(SizeIs(1),
                                               Contains(AllOf(HasId(0),IsReleased()))
                                               ),_)).Times(1);

        EXPECT_CALL(*mockWindowSystem,
                    handleTouchEvent(_,_,_,AllOf(SizeIs(1),
                                               Contains(AllOf(HasId(1),IsPressed()))
                                               ),_)).Times(1);
    }

    auto ev2 = mev::make_event(MirInputDeviceId(), std::chrono::milliseconds(125), std::vector<uint8_t>{} /* cookie */, 0);
    mev::add_touch(*ev2, 1 /* touch ID */, mir_touch_action_down, mir_touch_tooltype_unknown,
                   20, 20, 10 /* x, y, pressure*/,
                   1, 1, 10 /* touch major, minor, size */);
    qtEventFeeder->dispatch(*ev2);

    ASSERT_TRUE(Mock::VerifyAndClearExpectations(mockWindowSystem));
}

TEST_F(QtEventFeederTest, GenerateMissingTouchEnd2)
{

    setIrrelevantMockWindowSystemExpectations();

    auto ev1 = mev::make_event(MirInputDeviceId(), std::chrono::milliseconds(123), std::vector<uint8_t>{} /* cookie */, 0);
    mev::add_touch(*ev1, 0 /* touch ID */, mir_touch_action_down, mir_touch_tooltype_unknown,
                   10, 10, 10 /* x, y, pressure*/,
                   1, 1, 10 /* touch major, minor, size */);

    EXPECT_CALL(*mockWindowSystem, handleTouchEvent(_,_,_,AllOf(SizeIs(1),
                                                              Contains(AllOf(HasId(0),
                                                                             IsPressed()))),_)).Times(1);
    qtEventFeeder->dispatch(*ev1);

    ASSERT_TRUE(Mock::VerifyAndClearExpectations(mockWindowSystem));

    // ---

    setIrrelevantMockWindowSystemExpectations();

    auto ev2 = mev::make_event(MirInputDeviceId(), std::chrono::milliseconds(124), std::vector<uint8_t>{} /* cookie */, 0);
    mev::add_touch(*ev2, 0 /* touch ID */, mir_touch_action_change, mir_touch_tooltype_unknown,
                   10, 10, 10 /* x, y, pressure*/,
                   1, 1, 10 /* touch major, minor, size */);
    mev::add_touch(*ev2, 1 /* touch ID */, mir_touch_action_down, mir_touch_tooltype_unknown,
                   20, 20, 10 /* x, y, pressure*/,
                   1, 1, 10 /* touch major, minor, size */);

    EXPECT_CALL(*mockWindowSystem,
        handleTouchEvent(_,_,_,AllOf(SizeIs(2),
                                   Contains(AllOf(HasId(0), StateIsMoved())),
                                   Contains(AllOf(HasId(1), IsPressed()))
                                   ),_)).Times(1);
    qtEventFeeder->dispatch(*ev2);

    ASSERT_TRUE(Mock::VerifyAndClearExpectations(mockWindowSystem));

    // ---

    setIrrelevantMockWindowSystemExpectations();

    // touch 0 disappeared and touch 2 got pressed
    auto ev3 = mev::make_event(MirInputDeviceId(), std::chrono::milliseconds(125), std::vector<uint8_t>{} /* cookie */, 0);
    mev::add_touch(*ev3, 1 /* touch ID */, mir_touch_action_change, mir_touch_tooltype_unknown,
                   20, 20, 10 /* x, y, pressure*/,
                   1, 1, 10 /* touch major, minor, size */);
    mev::add_touch(*ev3, 2 /* touch ID */, mir_touch_action_down, mir_touch_tooltype_unknown,
                   30, 30, 10 /* x, y, pressure*/,
                   1, 1, 10 /* touch major, minor, size */);

    // There can be only one pressed or released touch per event
    {
        InSequence sequence;

        // first release touch 0
        EXPECT_CALL(*mockWindowSystem,
            handleTouchEvent(_,_,_,AllOf(SizeIs(2),
                                       Contains(AllOf(HasId(0), IsReleased())),
                                       Contains(AllOf(HasId(1), IsStationary()))
                                       ),_)).Times(1);

        // then press touch 2
        EXPECT_CALL(*mockWindowSystem,
            handleTouchEvent(_,_,_,AllOf(SizeIs(2),
                                       Contains(AllOf(HasId(1), StateIsMoved())),
                                       Contains(AllOf(HasId(2), IsPressed()))
                                       ),_)).Times(1);

    }
    qtEventFeeder->dispatch(*ev3);

    ASSERT_TRUE(Mock::VerifyAndClearExpectations(mockWindowSystem));
}

TEST_F(QtEventFeederTest, PressSameTouchTwice)
{
    setIrrelevantMockWindowSystemExpectations();

    EXPECT_CALL(*mockWindowSystem, handleTouchEvent(_,_,_,AllOf(SizeIs(1),
                                                              Contains(AllOf(HasId(0),
                                                                             IsPressed()))),_)).Times(1);

    auto ev1 = mev::make_event(MirInputDeviceId(), std::chrono::milliseconds(123), std::vector<uint8_t>{} /* cookie */, 0);
    mev::add_touch(*ev1, /* touch ID */ 0, mir_touch_action_down, mir_touch_tooltype_unknown,
                   10, 10, 10, 1, 1, 10);
    qtEventFeeder->dispatch(*ev1);

    ASSERT_TRUE(Mock::VerifyAndClearExpectations(mockWindowSystem));

    setIrrelevantMockWindowSystemExpectations();

    EXPECT_CALL(*mockWindowSystem, handleTouchEvent(_,_,_,AllOf(SizeIs(1),
                                                              Contains(AllOf(HasId(0), StateIsMoved()))
                                                             ),_)).Times(1);

    auto ev2 = mev::make_event(MirInputDeviceId(), std::chrono::milliseconds(123), std::vector<uint8_t>{} /* cookie */, 0);
    mev::add_touch(*ev2, /* touch ID */ 0, mir_touch_action_down, mir_touch_tooltype_unknown,
                   10, 10, 10, 1, 1, 10);
    qtEventFeeder->dispatch(*ev2);

    ASSERT_TRUE(Mock::VerifyAndClearExpectations(mockWindowSystem));
}

TEST_F(QtEventFeederTest, TimestampInMilliseconds)
{
    setIrrelevantMockWindowSystemExpectations();
    EXPECT_CALL(*mockWindowSystem, handleTouchEvent(_,0,_,_,_)).Times(1);
    auto ev1 = mev::make_event(MirInputDeviceId(), std::chrono::milliseconds(123), std::vector<uint8_t>{} /* cookie */, 0);
    qtEventFeeder->dispatch(*ev1);
    ASSERT_TRUE(Mock::VerifyAndClearExpectations(mockWindowSystem));

    setIrrelevantMockWindowSystemExpectations();
    EXPECT_CALL(*mockWindowSystem, handleTouchEvent(_,2,_,_,_)).Times(1);
    auto ev2 = mev::make_event(MirInputDeviceId(), std::chrono::milliseconds(125), std::vector<uint8_t>{} /* cookie */, 0);
    qtEventFeeder->dispatch(*ev2);
    ASSERT_TRUE(Mock::VerifyAndClearExpectations(mockWindowSystem));
}

TEST_F(QtEventFeederTest, composeKeysNotFilteredOut)
{
    auto down = mir_keyboard_action_down;
    auto up = mir_keyboard_action_up;

    auto dispatch_key_event = [&](MirKeyboardAction action, int scan_code, xkb_keysym_t key_sym)
    {
        qtEventFeeder->dispatch(*mev::make_event(
                MirInputDeviceId{0}, std::chrono::nanoseconds{0}, std::vector<uint8_t>{},
                action, key_sym, scan_code, mir_input_event_modifier_none));
    };

    InSequence seq;
    EXPECT_CALL(
        *mockWindowSystem,
        handleExtendedKeyEvent(_, _, QEvent::KeyPress, _, _, KEY_RIGHTSHIFT,
                               XKB_KEY_Shift_R, _, _, _));
    EXPECT_CALL(*mockWindowSystem,
                handleExtendedKeyEvent(_, _, QEvent::KeyPress, _, _,
                                       KEY_APOSTROPHE, XKB_KEY_NoSymbol,
                                       _, _, _));
    EXPECT_CALL(*mockWindowSystem,
                handleExtendedKeyEvent(_, _, QEvent::KeyRelease, _, _,
                                       KEY_APOSTROPHE, XKB_KEY_NoSymbol,
                                       _, _, _));
    EXPECT_CALL(
        *mockWindowSystem,
        handleExtendedKeyEvent(_, _, QEvent::KeyRelease, _, _, KEY_RIGHTSHIFT,
                               XKB_KEY_Shift_R, _, _, _));
    EXPECT_CALL(*mockWindowSystem,
                handleExtendedKeyEvent(_, _, QEvent::KeyPress, _, _, KEY_U,
                                       XKB_KEY_udiaeresis, _, _, _));
    dispatch_key_event(down, KEY_RIGHTSHIFT, XKB_KEY_Shift_R);
    dispatch_key_event(down, KEY_APOSTROPHE, XKB_KEY_NoSymbol);
    dispatch_key_event(up, KEY_APOSTROPHE, XKB_KEY_NoSymbol);
    dispatch_key_event(up, KEY_RIGHTSHIFT, XKB_KEY_Shift_R);
    dispatch_key_event(down, KEY_U, XKB_KEY_udiaeresis);
}
