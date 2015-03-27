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

#define MIR_INCLUDE_DEPRECATED_EVENT_HEADER

#include <gtest/gtest.h>

#include <QLoggingCategory>
#include <QTest>

// the test subject
#include <mirsurfaceitem.h>

// mocks
#include <mock_surface.h>
#include <mock_session.h>

using namespace qtmir;

using mir::scene::MockSurface;
using qtmir::MockSession;

// gtest stuff
using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::InSequence;
using ::testing::Return;

/*
  Tests that even if Qt fails to finish a touch sequence, MirSurfaceItem will
  properly finish it when forwarding it to its mir::input::surface. So
  mir::input::surface will still consume a proper sequence of touch events
  (comprised of a begin, zero or more updates and an end).
 */
TEST(MirSurfaceItemTest, MissingTouchEnd)
{
    // We don't want the logging spam cluttering the test results
    QLoggingCategory::setFilterRules(QStringLiteral("qtmir*=false"));

    std::shared_ptr<MockSurface> mockSurface = std::make_shared<MockSurface>();
    MockSession *mockSession = new MockSession;

    // Set some expectations and behavior for calls we are not interested in
    EXPECT_CALL(*mockSurface, add_observer(_)).Times(AnyNumber());
    EXPECT_CALL(*mockSurface, remove_observer(_)).Times(AnyNumber());
    EXPECT_CALL(*mockSurface, size()).Times(AnyNumber()).WillRepeatedly(Return(mir::geometry::Size(100,100)));
    EXPECT_CALL(*mockSurface, type()).Times(AnyNumber()).WillRepeatedly(Return(mir_surface_type_normal));
    EXPECT_CALL(*mockSession, setSurface(_)).Times(AnyNumber());

    auto getTouchEvent = [](MirEvent const& event) -> MirTouchEvent const*
    {
        if (mir_event_get_type(&event) != mir_event_type_input)
            return nullptr;
        auto const* input_event = mir_event_get_input_event(&event);
        if (mir_input_event_get_type(input_event) != mir_input_event_type_touch)
            return nullptr;
        return mir_input_event_get_touch_event(input_event);
    };

    auto eventMatches = [&](MirEvent const& event,
                            int touch_count,
                            MirTouchAction action,
                            MirTouchId touch_id) ->void
    {
        auto const* touch_event = getTouchEvent(event);
        ASSERT_NE(touch_event, nullptr);
        ASSERT_EQ(touch_count, mir_touch_event_point_count(touch_event));
        ASSERT_EQ(action, mir_touch_event_action(touch_event,0));
        ASSERT_EQ(touch_id, mir_touch_event_id(touch_event,0));
    };

    // The touch event sequence we expect mir::input::surface to receive from MirSurfaceItem.
    // It should properly finish the sequence for touch 0 ('down', 'move' and 'up') before starting
    // the sequence for touch 1.
    EXPECT_CALL(*mockSurface, consume(_))
        .WillOnce(Invoke([&] (MirEvent const& mirEvent) {
            eventMatches(mirEvent, 1, mir_touch_action_down, 0);
        }))
        .WillOnce(Invoke([&] (MirEvent const& mirEvent) {
            eventMatches(mirEvent, 1, mir_touch_action_change, 0);
        }))
        .WillOnce(Invoke([&] (MirEvent const& mirEvent) {
            eventMatches(mirEvent, 1, mir_touch_action_up, 0);
        }))
        .WillOnce(Invoke([&] (MirEvent const& mirEvent) {
            eventMatches(mirEvent, 1, mir_touch_action_down, 1);
        }));


    MirSurfaceItem *surfaceItem = new MirSurfaceItem(mockSurface, mockSession, nullptr, nullptr);

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

    delete surfaceItem;
    delete mockSession;
}
