/*
 * Copyright (C) 2015-2016 Canonical, Ltd.
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

#ifndef FAKE_MIRSURFACEINTERFACE_H
#define FAKE_MIRSURFACEINTERFACE_H

#include <Unity/Application/mirsurfaceinterface.h>
#include <Unity/Application/mirsurfacelistmodel.h>

#include <QSharedPointer>
#include <QSGTexture>
#include <QPointer>

namespace qtmir {

class FakeMirSurface : public MirSurfaceInterface
{
    Q_OBJECT

public:

    class TouchEvent {
    public:
        TouchEvent(Qt::KeyboardModifiers mods,
                const QList<QTouchEvent::TouchPoint> &points,
                Qt::TouchPointStates states,
                ulong timestamp);
        virtual ~TouchEvent();

        Qt::KeyboardModifiers keyboardModifiers;
        QList<QTouchEvent::TouchPoint> touchPoints;
        Qt::TouchPointStates states;
        ulong timestamp;
    };

    FakeMirSurface(QObject *parent = nullptr);
    virtual ~FakeMirSurface();

    ////
    // unity.shell.application.MirSurfaceInterface
    Mir::Type type() const override;
    QString name() const override;
    QString persistentId() const override;
    QSize size() const override;
    void resize(int width, int height) override;
    void resize(const QSize &size) override;
    Mir::State state() const override;
    void setState(Mir::State qmlState) override;
    bool live() const override;
    bool visible() const override;
    Mir::OrientationAngle orientationAngle() const override;
    void setOrientationAngle(Mir::OrientationAngle angle) override;

    int minimumWidth() const override { return 0; }
    int minimumHeight() const override { return 0; }
    int maximumWidth() const override { return 0; }
    int maximumHeight() const override { return 0; }
    int widthIncrement() const override { return 0; }
    int heightIncrement() const override { return 0; }

    void setKeymap(const QString &) override {}
    QString keymap() const override { return QString(); }

    Mir::ShellChrome shellChrome() const override { return Mir::NormalChrome; }

    bool focused() const override { return false; }
    QRect inputBounds() const override { return QRect(0,0,10,10); }
    bool confinesMousePointer() const override { return false; }

    void requestFocus() override {
        Q_EMIT focusRequested();
    }

    void close() override {
        Q_EMIT closeRequested();
    }

    void raise() override {}

    ////
    // qtmir.MirSurfaceInterface

    bool isFirstFrameDrawn() const override;
    void stopFrameDropper() override;
    void startFrameDropper() override;
    void setLive(bool value) override;
    void setViewVisibility(qintptr viewId, bool visible) override;
    bool isBeingDisplayed() const override;
    void registerView(qintptr viewId) override;
    void unregisterView(qintptr viewId) override;

    // methods called from the rendering (scene graph) thread:
    QSharedPointer<QSGTexture> texture() override;
    QSGTexture *weakTexture() const override;
    bool updateTexture() override;
    unsigned int currentFrameNumber() const override;
    bool numBuffersReadyForCompositor() override;
    // end of methods called from the rendering (scene graph) thread

    void setFocused(bool focus) override;

    void setViewActiveFocus(qintptr, bool) override {};
    bool activeFocus() const override { return false; }

    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void hoverEnterEvent(QHoverEvent *) override;
    void hoverLeaveEvent(QHoverEvent *) override;
    void hoverMoveEvent(QHoverEvent *) override;
    void wheelEvent(QWheelEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;

    void touchEvent(Qt::KeyboardModifiers mods,
            const QList<QTouchEvent::TouchPoint> &points,
            Qt::TouchPointStates states,
            ulong timestamp) override;

    QString appId() const override;

    QCursor cursor() const override { return QCursor(); }

    void setScreen(QScreen *) override {}

    SessionInterface* session() override { return m_session; }

    bool inputAreaContains(const QPoint &) const override { return true; }

public Q_SLOTS:
    void onCompositorSwappedBuffers() override;

    void setMinimumWidth(int) override {}
    void setMinimumHeight(int) override {}
    void setMaximumWidth(int) override {}
    void setMaximumHeight(int) override {}
    void setWidthIncrement(int) override {}
    void setHeightIncrement(int) override {}
    void setShellChrome(Mir::ShellChrome) override {}

    ////
    // Test API from now on

public:

    void drawFirstFrame();

    bool isFrameDropperRunning() const;

    QList<TouchEvent> &touchesReceived();

    void setSession(SessionInterface *session);

private:
    void updateVisibility();


    bool m_isFirstFrameDrawn;
    bool m_isFrameDropperRunning;
    bool m_live;
    Mir::State m_state;
    Mir::OrientationAngle m_orientationAngle;
    bool m_visible;
    QSize m_size;
    QHash<int, bool> m_views;
    bool m_focused;

    QList<TouchEvent> m_touchesReceived;

    SessionInterface *m_session{nullptr};
};

} // namespace qtmir

#endif // FAKE_MIRSURFACEINTERFACE_H
