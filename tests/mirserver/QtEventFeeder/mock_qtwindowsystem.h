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

#ifndef MOCK_QTWINDOWSYSTEM_H
#define MOCK_QTWINDOWSYSTEM_H

#include <qteventfeeder.h>

class MockQtWindowSystem : public QtEventFeeder::QtWindowSystemInterface {
public:
    MOCK_METHOD0(hasTargetWindow, bool());
    MOCK_METHOD0(targetWindowGeometry, QRect());
    MOCK_METHOD1(registerTouchDevice, void(QTouchDevice* device));
    MOCK_METHOD10(handleExtendedKeyEvent, void(ulong timestamp, QEvent::Type type, int key,
            Qt::KeyboardModifiers modifiers,
            quint32 nativeScanCode, quint32 nativeVirtualKey,
            quint32 nativeModifiers,
            const QString& text, bool autorep,
            ushort count));
    MOCK_METHOD4(handleTouchEvent, void(ulong timestamp, QTouchDevice *device,
            const QList<struct QWindowSystemInterface::TouchPoint> &points,
            Qt::KeyboardModifiers mods));
    MOCK_METHOD4(handleMouseEvent, void(ulong timestamp, QPointF point, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers));
    MOCK_METHOD7(handleWheelEvent, void(ulong timestamp, const QPointF & local, const QPointF & global, QPoint pixelDelta, QPoint angleDelta,
                                        Qt::KeyboardModifiers mods, Qt::ScrollPhase phase));
    MOCK_METHOD2(handleEnterEvent, void(const QPointF &localPoint, const QPointF& globalPoint));
    MOCK_METHOD0(handleLeaveEvent, void());
};

namespace testing
{

MATCHER(IsPressed, std::string(negation ? "isn't" : "is") + " pressed")
{
    return arg.state == Qt::TouchPointPressed;
}

MATCHER(IsReleased, std::string(negation ? "isn't" : "is") + " released")
{
    return arg.state == Qt::TouchPointReleased;
}

MATCHER(IsStationary, std::string(negation ? "isn't" : "is") + " stationary")
{
    return arg.state == Qt::TouchPointStationary;
}

MATCHER(StateIsMoved, "state " + std::string(negation ? "isn't" : "is") + " 'moved'")
{
    return arg.state == Qt::TouchPointMoved;
}

MATCHER_P(HasId, expectedId, "id " + std::string(negation ? "isn't " : "is ") + PrintToString(expectedId))
{
    return arg.id == expectedId;
}

} // namespace testing


#endif // MOCK_QTWINDOWSYSTEM_H
