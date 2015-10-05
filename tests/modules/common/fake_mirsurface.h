/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include <QSharedPointer>
#include <QSGTexture>

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
                ulong timestamp)
            : keyboardModifiers(mods)
            , touchPoints(points)
            , states(states)
            , timestamp(timestamp) {}

        Qt::KeyboardModifiers keyboardModifiers;
        QList<QTouchEvent::TouchPoint> touchPoints;
        Qt::TouchPointStates states;
        ulong timestamp;
    };


    FakeMirSurface(QObject *parent = nullptr)
        : MirSurfaceInterface(parent)
        , m_isFirstFrameDrawn(false)
        , m_session(nullptr)
        , m_isFrameDropperRunning(true)
        , m_live(true)
        , m_state(Mir::RestoredState)
        , m_orientationAngle(Mir::Angle0)
        , m_visible(true)
        , m_viewCount(0)
        , m_focused(false)
    {}

    ////
    // unity.shell.application.MirSurfaceInterface
    Mir::Type type() const override { return Mir::NormalType; }

    QString name() const { return QString("Fake MirSurface"); }

    QSize size() const override { return m_size; }

    void resize(int width, int height) override {
        if (m_size.width() != width || m_size.height() != height) {
            m_size.setWidth(width);
            m_size.setHeight(height);
            Q_EMIT sizeChanged(m_size);
        }
    }
    void resize(const QSize &size) override { resize(size.width(), size.height()); }

    Mir::State state() const override { return m_state; }
    void setState(Mir::State qmlState) override {
        if (qmlState != m_state) {
            m_state = qmlState;
            Q_EMIT stateChanged(m_state);
        }
    }

    bool live() const override { return m_live; }

    bool visible() const override { return m_visible; }

    Mir::OrientationAngle orientationAngle() const override { return m_orientationAngle; }
    void setOrientationAngle(Mir::OrientationAngle angle) override {
        if (m_orientationAngle != angle) {
            m_orientationAngle = angle;
            Q_EMIT orientationAngleChanged(m_orientationAngle);
        }
    }

    ////
    // qtmir.MirSurfaceInterface

    bool isFirstFrameDrawn() const override {
        return m_isFirstFrameDrawn;
    }

    void stopFrameDropper() override {
        m_isFrameDropperRunning = false;
    }
    void startFrameDropper() override {
        m_isFrameDropperRunning = true;
    }

    void setLive(bool value) override {
        if (m_live != value) {
            m_live = value;
            Q_EMIT liveChanged(m_live);
        }
    }

    void setVisible(bool visible) override {
        if (m_visible != visible) {
            m_visible = visible;
            Q_EMIT visibleChanged(m_visible);
        }
    }

    bool isBeingDisplayed() const override { return m_viewCount > 0; }
    void incrementViewCount() override {
        ++m_viewCount;
        if (m_viewCount == 1) {
            Q_EMIT isBeingDisplayedChanged();
        }
    }
    void decrementViewCount() override {
        --m_viewCount;
        if (m_viewCount == 0) {
            Q_EMIT isBeingDisplayedChanged();
        }
    }

    // methods called from the rendering (scene graph) thread:
    QSharedPointer<QSGTexture> texture() override { return QSharedPointer<QSGTexture>(); }
    QSGTexture *weakTexture() const override { return nullptr; }
    void updateTexture() override {}
    unsigned int currentFrameNumber() const override { return 0; }
    bool numBuffersReadyForCompositor() override { return 0; }
    // end of methods called from the rendering (scene graph) thread

    void setFocus(bool focus) override { m_focused = focus; }

    void mousePressEvent(QMouseEvent *) override {}
    void mouseMoveEvent(QMouseEvent *) override {}
    void mouseReleaseEvent(QMouseEvent *) override {}
    void hoverEnterEvent(QHoverEvent *) override {}
    void hoverLeaveEvent(QHoverEvent *) override {}
    void hoverMoveEvent(QHoverEvent *) override {}

    void keyPressEvent(QKeyEvent *) override {}
    void keyReleaseEvent(QKeyEvent *) override {}

    void touchEvent(Qt::KeyboardModifiers mods,
            const QList<QTouchEvent::TouchPoint> &points,
            Qt::TouchPointStates states,
            ulong timestamp) override {
        m_touchesReceived.append(TouchEvent(mods, points, states, timestamp));
    }

    QString appId() const override { return "foo-app"; }

public Q_SLOTS:
    void onCompositorSwappedBuffers() override {}

    ////
    // Test API from now on

public:

    void drawFirstFrame() {
        if (!m_isFirstFrameDrawn) {
            m_isFirstFrameDrawn = true;
            Q_EMIT firstFrameDrawn();
        }
    }

    bool isFrameDropperRunning() const {
        return m_isFrameDropperRunning;
    }

    QList<TouchEvent> &touchesReceived() { return m_touchesReceived; }

private:

    bool m_isFirstFrameDrawn;
    SessionInterface *m_session;
    bool m_isFrameDropperRunning;
    bool m_live;
    Mir::State m_state;
    Mir::OrientationAngle m_orientationAngle;
    bool m_visible;
    QSize m_size;
    int m_viewCount;
    bool m_focused;

    QList<TouchEvent> m_touchesReceived;
};

} // namespace qtmir

#endif // FAKE_MIRSURFACEINTERFACE_H
