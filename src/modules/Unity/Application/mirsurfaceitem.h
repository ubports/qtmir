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
#include <QSet>
#include <QQuickItem>
#include <QTimer>
#include <QQmlListProperty>

// mir
#include <mir/scene/surface.h>
#include <mir_toolkit/common.h>

#include "session_interface.h"

namespace mir { namespace shell { class Shell; }}

class SurfaceObserver;
using MirShell = mir::shell::Shell;

namespace qtmir {

class MirSurfaceManager;
class QSGMirSurfaceNode;
class QMirSurfaceTextureProvider;
class Application;

class MirSurfaceItem : public QQuickItem
{
    Q_OBJECT
    Q_ENUMS(Type)
    Q_ENUMS(State)
    Q_ENUMS(OrientationAngle)

    Q_PROPERTY(Type type READ type NOTIFY typeChanged)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(bool live READ live NOTIFY liveChanged)

    // How many degrees, clockwise, the UI in the surface has to rotate to match with the
    // shell UI orientation
    Q_PROPERTY(OrientationAngle orientationAngle READ orientationAngle WRITE setOrientationAngle
               NOTIFY orientationAngleChanged DESIGNABLE false)

public:
    explicit MirSurfaceItem(std::shared_ptr<mir::scene::Surface> surface,
                            SessionInterface* session,
                            MirShell *shell,
                            std::shared_ptr<SurfaceObserver> observer,
                            QQuickItem *parent = 0);
    ~MirSurfaceItem();

    enum Type {
        Normal = mir_surface_type_normal,
        Utility = mir_surface_type_utility,
        Dialog = mir_surface_type_dialog,
        Overlay = mir_surface_type_overlay,
        Freestyle = mir_surface_type_freestyle,
        Popover = mir_surface_type_popover,
        InputMethod = mir_surface_type_inputmethod,
        };

    enum State {
        Unknown = mir_surface_state_unknown,
        Restored = mir_surface_state_restored,
        Minimized = mir_surface_state_minimized,
        Maximized = mir_surface_state_maximized,
        VertMaximized = mir_surface_state_vertmaximized,
        /* SemiMaximized = mir_surface_state_semimaximized, // see mircommon/mir_toolbox/common.h*/
        Fullscreen = mir_surface_state_fullscreen,
    };

    enum OrientationAngle {
        Angle0 = 0,
        Angle90 = 90,
        Angle180 = 180,
        Angle270 = 270
    };

    //getters
    Type type() const;
    State state() const;
    QString name() const;
    bool live() const;
    SessionInterface *session() const;

    Q_INVOKABLE void release();

    // Item surface/texture management
    bool isTextureProvider() const { return true; }
    QSGTextureProvider *textureProvider() const;

    void stopFrameDropper();
    void startFrameDropper();

    bool isFirstFrameDrawn() const { return m_firstFrameDrawn; }

    OrientationAngle orientationAngle() const;
    void setOrientationAngle(OrientationAngle angle);

    void setSession(SessionInterface *app);

    // to allow easy touch event injection from tests
    bool processTouchEvent(int eventType,
            ulong timestamp,
            Qt::KeyboardModifiers modifiers,
            const QList<QTouchEvent::TouchPoint> &touchPoints,
            Qt::TouchPointStates touchPointStates);

Q_SIGNALS:
    void typeChanged();
    void stateChanged();
    void nameChanged();
    void orientationAngleChanged(OrientationAngle angle);
    void liveChanged(bool live);
    void firstFrameDrawn(MirSurfaceItem *item);

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
    void dropPendingBuffers();
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
    void setLive(const bool);

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

    friend class MirSurfaceManager;
};

} // namespace qtmir

Q_DECLARE_METATYPE(qtmir::MirSurfaceItem*)
Q_DECLARE_METATYPE(qtmir::MirSurfaceItem::OrientationAngle)

#endif // MIRSURFACEITEM_H
