/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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

#ifndef MIRSURFACEITEM_H
#define MIRSURFACEITEM_H

#include <memory>

// Qt
#include <QMutex>
#include <QPointer>
#include <QTimer>
#include <QQmlListProperty>

// mir
#include <mir/scene/surface.h>
#include <mir_toolkit/common.h>

#include "mirsurfaceiteminterface.h"
#include "session_interface.h"

namespace mir { namespace shell { class Shell; }}

class SurfaceObserver;
using MirShell = mir::shell::Shell;

namespace qtmir {

class MirSurfaceManager;
class QSGMirSurfaceNode;
class QMirSurfaceTextureProvider;

class MirSurfaceItem : public MirSurfaceItemInterface
{
    Q_OBJECT

public:
    explicit MirSurfaceItem(std::shared_ptr<mir::scene::Surface> surface,
                            SessionInterface* session,
                            MirShell *shell,
                            std::shared_ptr<SurfaceObserver> observer,
                            QQuickItem *parent = 0);
    virtual ~MirSurfaceItem();

    //getters
    Type type() const override;
    State state() const override;
    QString name() const override;
    bool live() const override;
    SessionInterface *session() const override;
    OrientationAngle orientationAngle() const override;

    Q_INVOKABLE void release() override;

    // Item surface/texture management
    bool isTextureProvider() const { return true; }
    QSGTextureProvider *textureProvider() const;

    void stopFrameDropper() override;
    void startFrameDropper() override;

    bool isFirstFrameDrawn() const override { return m_firstFrameDrawn; }

    void setOrientationAngle(OrientationAngle angle) override;
    void setSession(SessionInterface *app) override;

    // to allow easy touch event injection from tests
    bool processTouchEvent(int eventType,
            ulong timestamp,
            Qt::KeyboardModifiers modifiers,
            const QList<QTouchEvent::TouchPoint> &touchPoints,
            Qt::TouchPointStates touchPointStates);

protected Q_SLOTS:
    void onSessionStateChanged(SessionInterface::State state);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void hoverEnterEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void touchEvent(QTouchEvent *event) override;

    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);

private Q_SLOTS:
    void surfaceDamaged();
    void dropPendingBuffer();
    void scheduleTextureUpdate();

    void scheduleMirSurfaceSizeUpdate();
    void updateMirSurfaceSize();

    void updateMirSurfaceFocus(bool focused);
    void onAttributeChanged(const MirSurfaceAttrib, const int);

private:
    bool updateTexture();
    void ensureProvider();

    void setType(const Type&);
    void setState(const State&);
    void setLive(bool) override;

    // called by MirSurfaceManager
    void setSurfaceValid(const bool);

    bool hasTouchInsideUbuntuKeyboard(const QList<QTouchEvent::TouchPoint> &touchPoints);
    bool isMouseInsideUbuntuKeyboard(const QMouseEvent *event);
    void syncSurfaceSizeWithItemSize();

    bool clientIsRunning() const;

    QString appId() const;
    void endCurrentTouchSequence(ulong timestamp);
    void validateAndDeliverTouchEvent(int eventType,
            ulong timestamp,
            Qt::KeyboardModifiers modifiers,
            const QList<QTouchEvent::TouchPoint> &touchPoints,
            Qt::TouchPointStates touchPointStates);

    QMutex m_mutex;

    std::shared_ptr<mir::scene::Surface> m_surface;
    QPointer<SessionInterface> m_session;
    MirShell *const m_shell;
    bool m_firstFrameDrawn;
    bool m_live;

    //FIXME -  have to save the state as Mir has no getter for it (bug:1357429)
    OrientationAngle m_orientationAngle;

    QMirSurfaceTextureProvider *m_textureProvider;

    std::shared_ptr<SurfaceObserver> m_surfaceObserver;

    QTimer m_frameDropperTimer;

    QTimer m_updateMirSurfaceSizeTimer;

    class TouchEvent {
    public:
        TouchEvent &operator= (const QTouchEvent &qtEvent) {
            type = qtEvent.type();
            timestamp = qtEvent.timestamp();
            modifiers = qtEvent.modifiers();
            touchPoints = qtEvent.touchPoints();
            touchPointStates = qtEvent.touchPointStates();
            return *this;
        }

        void updateTouchPointStatesAndType();

        int type;
        ulong timestamp;
        Qt::KeyboardModifiers modifiers;
        QList<QTouchEvent::TouchPoint> touchPoints;
        Qt::TouchPointStates touchPointStates;
    } *m_lastTouchEvent;
};

} // namespace qtmir

#endif // MIRSURFACEITEM_H
