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
#include <QWindow>

class MockQtWindowSystem : public QtEventFeeder::QtWindowSystemInterface {
public:
    MOCK_CONST_METHOD0(ready, bool());
    MOCK_METHOD1(setScreenController, void(const QSharedPointer<ScreenController> &));
    MOCK_METHOD1(getWindowForTouchPoint, QWindow*(const QPoint &point));
    MOCK_METHOD0(lastWindow, QWindow*());
    MOCK_METHOD0(focusedWindow, QWindow*());
    MOCK_METHOD1(registerTouchDevice, void(QTouchDevice* device));

    // Wanted to use GMock, but MOCK_METHOD11 not implemented
    void handleExtendedKeyEvent(QWindow */*window*/, ulong /*timestamp*/, QEvent::Type /*type*/, int /*key*/,
            Qt::KeyboardModifiers /*modifiers*/,
            quint32 /*nativeScanCode*/, quint32 /*nativeVirtualKey*/,
            quint32 /*nativeModifiers*/,
            const QString& /*text*/ = QString(), bool /*autorep*/ = false,
            ushort /*count*/ = 1) {}

    MOCK_METHOD5(handleTouchEvent, void(QWindow *window, ulong timestamp, QTouchDevice *device,
            const QList<struct QWindowSystemInterface::TouchPoint> &points,
            Qt::KeyboardModifiers mods));
    MOCK_METHOD4(handleMouseEvent, void(ulong timestamp, QPointF point, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers));
    MOCK_METHOD3(handleWheelEvent, void(ulong timestamp, QPoint angleDelta, Qt::KeyboardModifiers mods));
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
