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

// the test subject
#include <Unity/Application/mirsurfaceitem.h>

// tests/framework
#include <fake_mirsurface.h>

using namespace qtmir;

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
