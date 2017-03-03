/*
 * Copyright (C) 2016-2017 Canonical, Ltd.
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

#include <eventbuilder.h>

#include <QScopedPointer>

#include "mir/events/event_builders.h"
#include "mir_toolkit/mir_cookie.h"

using namespace qtmir;

class EventBuilderTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/*
 For now all we care about is whether the relative pointer axes get carried over to the reconstructed event
 */
TEST_F(EventBuilderTest, ReconstructPointerEvent)
{
    QScopedPointer<EventBuilder> eventBuilder(new EventBuilder);

    float relativeX = 1.1;
    float relativeY = 2.2;

    ulong qtTimestamp = 12345;

    // Store the event whose data we will get later when reconstructing it.
    {
        mir::EventUPtr mirEvent = mir::events::make_event(0 /*DeviceID */, std::chrono::nanoseconds(111)/*timestamp*/,
                std::vector<uint8_t>{} /* cookie */, mir_input_event_modifier_none, mir_pointer_action_motion, 0 /*buttons*/,
                0 /*x*/, 0 /*y*/, 0 /*hscroll*/, 0 /*vscroll*/, relativeX, relativeY);

        eventBuilder->store(mir_event_get_input_event(mirEvent.get()), qtTimestamp);
    }

    // And store some other unrelated events in between
    {
        mir::EventUPtr mirEvent = mir::events::make_event(0 /*DeviceID */, std::chrono::nanoseconds(222)/*timestamp*/,
            std::vector<uint8_t>{} /* cookie */, mir_input_event_modifier_none, mir_pointer_action_motion, 0 /*buttons*/,
            0 /*x*/, 0 /*y*/, 0 /*hscroll*/, 0 /*vscroll*/, relativeX + 3.3, relativeY + 3.3);
        eventBuilder->store(mir_event_get_input_event(mirEvent.get()), qtTimestamp + 10);
    }
    {
        mir::EventUPtr mirEvent = mir::events::make_event(0 /*DeviceID */, std::chrono::nanoseconds(333)/*timestamp*/,
            std::vector<uint8_t>{} /* cookie */, mir_input_event_modifier_none, mir_pointer_action_motion, 0 /*buttons*/,
            0 /*x*/, 0 /*y*/, 0 /*hscroll*/, 0 /*vscroll*/, relativeX + 4.4, relativeY + 4.4);
        eventBuilder->store(mir_event_get_input_event(mirEvent.get()), qtTimestamp + 20);
    }

    QMouseEvent mouseEvent(QEvent::MouseMove, QPointF(0,0) /*localPos*/, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    mouseEvent.setTimestamp(qtTimestamp); // this is what identifies this Qt event with the first Mir event stored.

    mir::EventUPtr newMirEvent = eventBuilder->reconstructMirEvent(&mouseEvent);

    // The reconstructed event should have the same relative axes values as the mir event stored with the same timestamp
    const MirPointerEvent *newMirPointerEvent = mir_input_event_get_pointer_event(mir_event_get_input_event(newMirEvent.get()));
    EXPECT_EQ(relativeX, mir_pointer_event_axis_value(newMirPointerEvent, mir_pointer_axis_relative_x));
    EXPECT_EQ(relativeY, mir_pointer_event_axis_value(newMirPointerEvent, mir_pointer_axis_relative_y));
}

TEST_F(EventBuilderTest, ReconstructCookies)
{
    QScopedPointer<EventBuilder> eventBuilder(new EventBuilder);

    std::vector<uint8_t> cookie{0xd7, 0x56, 0xf1, 0xb7, 0xd8, 0xba};

    ulong qtTimestamp = 12345;

    // Store the event whose data we will get later when reconstructing it.
    {
        mir::EventUPtr mirEvent = mir::events::make_event(0 /*DeviceID */, std::chrono::nanoseconds(111)/*timestamp*/,
                cookie, mir_input_event_modifier_none, mir_pointer_action_button_down, mir_pointer_button_primary/*buttons*/,
                0 /*x*/, 0 /*y*/, 0 /*hscroll*/, 0 /*vscroll*/, 0 /*relativeX*/, 0 /*relativeY*/);

        eventBuilder->store(mir_event_get_input_event(mirEvent.get()), qtTimestamp);
    }

    // And store some other unrelated events in between
    {
        mir::EventUPtr mirEvent = mir::events::make_event(0 /*DeviceID */, std::chrono::nanoseconds(222)/*timestamp*/,
            std::vector<uint8_t>{} /* cookie */, mir_input_event_modifier_none, mir_pointer_action_motion, 0 /*buttons*/,
            0 /*x*/, 0 /*y*/, 0 /*hscroll*/, 0 /*vscroll*/, 0 /*relativeX*/, 0 /*relativeY*/);
        eventBuilder->store(mir_event_get_input_event(mirEvent.get()), qtTimestamp + 10);
    }
    {
        mir::EventUPtr mirEvent = mir::events::make_event(0 /*DeviceID */, std::chrono::nanoseconds(333)/*timestamp*/,
            std::vector<uint8_t>{} /* cookie */, mir_input_event_modifier_none, mir_pointer_action_motion, 0 /*buttons*/,
            0 /*x*/, 0 /*y*/, 0 /*hscroll*/, 0 /*vscroll*/, 0 /*relativeX*/, 0 /*relativeY*/);
        eventBuilder->store(mir_event_get_input_event(mirEvent.get()), qtTimestamp + 20);
    }

    QMouseEvent mouseEvent(QEvent::MouseMove, QPointF(0,0) /*localPos*/, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    mouseEvent.setTimestamp(qtTimestamp); // this is what identifies this Qt event with the first Mir event stored.

    mir::EventUPtr newMirEvent = eventBuilder->reconstructMirEvent(&mouseEvent);

    // The reconstructed event should have the same relative axes values as the mir event stored with the same timestamp
    auto input_event = mir_event_get_input_event(newMirEvent.get());
    auto cookie_ptr = mir_input_event_get_cookie(input_event);
    std::vector<uint8_t> cookieFromPointerEvent;
    cookieFromPointerEvent.resize(mir_cookie_buffer_size(cookie_ptr));
    mir_cookie_to_buffer(cookie_ptr, cookieFromPointerEvent.data(), cookieFromPointerEvent.size());
    mir_cookie_release(cookie_ptr);
    EXPECT_EQ(cookieFromPointerEvent, cookie);
}

TEST_F(EventBuilderTest, ReconstructDeviceId)
{
    QScopedPointer<EventBuilder> eventBuilder(new EventBuilder);

    MirInputDeviceId deviceId = 17;
    ulong qtTimestamp = 12345;

    // Store the event whose data we will get later when reconstructing it.
    {
        mir::EventUPtr mirEvent = mir::events::make_event(deviceId, std::chrono::nanoseconds(111)/*timestamp*/,
                std::vector<uint8_t>{}/*cookie*/, mir_keyboard_action_down, 70, 50,
                mir_input_event_modifier_none);

        eventBuilder->store(mir_event_get_input_event(mirEvent.get()), qtTimestamp);
    }

    // And store some other unrelated events in between
    {
        mir::EventUPtr mirEvent = mir::events::make_event(0 /*DeviceID */, std::chrono::nanoseconds(222)/*timestamp*/,
            std::vector<uint8_t>{} /* cookie */, mir_input_event_modifier_none, mir_pointer_action_motion, 0 /*buttons*/,
            0 /*x*/, 0 /*y*/, 0 /*hscroll*/, 0 /*vscroll*/, 0 /*relativeX*/,0 /*relativeY*/);
        eventBuilder->store(mir_event_get_input_event(mirEvent.get()), qtTimestamp + 10);
    }

    QKeyEvent keyEvent(QEvent::KeyPress, 70, Qt::NoModifier);
    keyEvent.setTimestamp(qtTimestamp); // this is what identifies this Qt event with the first Mir event stored.

    mir::EventUPtr newMirEvent = eventBuilder->makeMirEvent(&keyEvent);

    // The reconstructed event should have the same relative axes values as the mir event stored with the same timestamp
    auto input_event = mir_event_get_input_event(newMirEvent.get());
    EXPECT_EQ(deviceId, mir_input_event_get_device_id(input_event));
}
