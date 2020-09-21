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
 *
 */

#ifndef MOCK_QTWINDOWSYSTEM_H
#define MOCK_QTWINDOWSYSTEM_H

#include <qteventfeeder.h>
#include <QGuiApplication>
#include <QWindow>

class MockQtWindowSystem : public QtEventFeeder::QtWindowSystemInterface {
public:
    MOCK_CONST_METHOD0(ready, bool());
    MOCK_METHOD0(lastWindow, QWindow*());
    MOCK_METHOD0(focusedWindow, QWindow*());
    // ignores the last parameter count, due to parameter limit in gmock
    MOCK_METHOD10(handleExtendedKeyEvent,
                  void(QWindow *window, ulong timestamp, QEvent::Type type,
                       int key, Qt::KeyboardModifiers modifiers,
                       quint32 nativeScanCode, quint32 nativeVirtualKey,
                       quint32 nativeModifiers, const QString &text,
                       bool autorep));

    void handleExtendedKeyEvent(QWindow *window, ulong timestamp, QEvent::Type type, int key,
            Qt::KeyboardModifiers modifiers,
            quint32 nativeScanCode, quint32 nativeVirtualKey,
            quint32 nativeModifiers,
            const QString& text, bool autorep,
            ushort /*count*/ )
    {
        handleExtendedKeyEvent(window, timestamp, type, key, modifiers,
                               nativeScanCode, nativeVirtualKey,
                               nativeModifiers, text, autorep);
    }

    MOCK_METHOD5(handleTouchEvent, void(QWindow *window, ulong timestamp, QTouchDevice *device,
            const QList<struct QWindowSystemInterface::TouchPoint> &points,
            Qt::KeyboardModifiers mods));

    ~MockQtWindowSystem()
    {
        qDeleteAll(m_devices);
    }

    void registerTouchDevice(QTouchDevice* device)
    {
        m_devices << device;
    }

    QWindow* getWindowForPoint(const QPoint&)
    {
        auto windows = qApp->topLevelWindows();
        return windows.isEmpty() ? nullptr : windows.first();
    }

    QVector<QTouchDevice*> m_devices;
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
