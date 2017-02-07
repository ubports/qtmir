/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
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

#ifndef MIR_QT_EVENT_FEEDER_H
#define MIR_QT_EVENT_FEEDER_H

#include <mir_toolkit/event.h>

#include <QObject>
#include <qpa/qwindowsysteminterface.h>

#include <memory>

class QTouchDevice;
class ScreensModel;

/*
  Fills Qt's event loop with input events from Mir
 */
class QtEventFeeder : public QObject
{
public:
    // Interface between QtEventFeeder and the actual QWindowSystemInterface functions
    // and other related Qt methods and objects to enable replacing them with mocks in
    // pure unit tests.
    class QtWindowSystemInterface {
        public:
        virtual ~QtWindowSystemInterface() {}
        virtual void setScreensModel(const std::shared_ptr<ScreensModel> &sc) = 0;
        virtual QWindow* getWindowForPoint(const QPoint &point) = 0;
        virtual QWindow* focusedWindow() = 0;
        virtual void registerTouchDevice(QTouchDevice *device) = 0;
        virtual void handleExtendedKeyEvent(QWindow *window, ulong timestamp, QEvent::Type type, int key,
                Qt::KeyboardModifiers modifiers,
                quint32 nativeScanCode, quint32 nativeVirtualKey,
                quint32 nativeModifiers,
                const QString& text = QString(), bool autorep = false,
                ushort count = 1) = 0;
        virtual void handleTouchEvent(QWindow *window, ulong timestamp, QTouchDevice *device,
                const QList<struct QWindowSystemInterface::TouchPoint> &points,
                Qt::KeyboardModifiers mods = Qt::NoModifier) = 0;
    };

    QtEventFeeder(const std::shared_ptr<ScreensModel> &screensModel);
    QtEventFeeder(const std::shared_ptr<ScreensModel> &screensModel,
                  QtWindowSystemInterface *windowSystem);
    virtual ~QtEventFeeder();

    void dispatchKey(MirKeyboardEvent const* event);
    void dispatchTouch(MirTouchEvent const* event);
    void dispatchPointer(MirPointerEvent const* event);

    bool dispatch(MirEvent const& event); // FIXME used only in tests

    bool event(QEvent *event) Q_DECL_OVERRIDE;

private:
    void validateTouches(QWindow *window, ulong timestamp, QList<QWindowSystemInterface::TouchPoint> &touchPoints);
    bool validateTouch(QWindowSystemInterface::TouchPoint &touchPoint);
    void sendActiveTouchRelease(QWindow *window, ulong timestamp, int id);

    QString touchesToString(const QList<struct QWindowSystemInterface::TouchPoint> &points);

    QTouchDevice *mTouchDevice;
    QtWindowSystemInterface *mQtWindowSystem;

    // Maps the id of an active touch to its last known state
    QHash<int, QWindowSystemInterface::TouchPoint> mActiveTouches;
    Qt::MouseButtons m_buttons;
};

#endif // MIR_QT_EVENT_FEEDER_H
