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

#ifndef QTMIR_MIRSURFACE_H
#define QTMIR_MIRSURFACE_H

#include "mirsurfaceinterface.h"
#include "mirsurfacelistmodel.h"

// Qt
#include <QCursor>
#include <QMutex>
#include <QPointer>
#include <QRect>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QSet>
#include <QTimer>

#include "mirbuffersgtexture.h"
#include "windowcontrollerinterface.h"
#include "windowmodelnotifier.h"

// mir
#include <mir_toolkit/common.h>


class SurfaceObserver;

namespace qtmir {

class AbstractTimer;
class MirSurfaceListModel;
class SessionInterface;
class CompositorTextureProvider;
class CompositorTexture;

class MirSurface : public MirSurfaceInterface
{
    Q_OBJECT

public:
    MirSurface(NewWindow windowInfo,
               WindowControllerInterface *controller,
               SessionInterface *session = nullptr,
               MirSurface *parentSurface = nullptr);
    virtual ~MirSurface();

    ////
    // unity::shell::application::MirSurfaceInterface

    Mir::Type type() const override;

    QString name() const override;

    QString persistentId() const override;

    QSize size() const override;
    void resize(int width, int height) override;
    Q_INVOKABLE void resize(const QSize &size) override { resize(size.width(), size.height()); }

    QPoint position() const override;

    QPoint requestedPosition() const override;
    void setRequestedPosition(const QPoint &) override;

    Mir::State state() const override;

    bool live() const override;

    bool visible() const override;

    Mir::OrientationAngle orientationAngle() const override;
    void setOrientationAngle(Mir::OrientationAngle angle) override;

    int minimumWidth() const override;
    int minimumHeight() const override;
    int maximumWidth() const override;
    int maximumHeight() const override;
    int widthIncrement() const override;
    int heightIncrement() const override;

    bool focused() const override;
    QRect inputBounds() const override;

    bool confinesMousePointer() const override;

    bool allowClientResize() const override;
    void setAllowClientResize(bool) override;

    Q_INVOKABLE void activate() override;

    unity::shell::application::MirSurfaceInterface *parentSurface() const override;
    unity::shell::application::MirSurfaceListInterface *childSurfaceList() const override;

    Q_INVOKABLE void close() override;

    ////
    // qtmir::MirSurfaceInterface

    void setLive(bool value) override;

    bool isReady() const override { return m_ready; }

    void stopFrameDropper() override;
    void startFrameDropper() override;

    bool isBeingDisplayed() const override;

    void registerView(qintptr viewId) override;
    void unregisterView(qintptr viewId) override;
    void setViewExposure(qintptr viewId, bool exposed) override;

    // methods called from the rendering (scene graph) thread:
    QSharedPointer<QSGTexture> texture(qintptr userId) override;
    QSGTexture *weakTexture(qintptr userId) const override;
    bool updateTexture(qintptr userId) override;
    unsigned int currentFrameNumber(qintptr userId) const override;
    bool numBuffersReadyForCompositor(qintptr userId) override;
    // end of methods called from the rendering (scene graph) thread

    void setFocused(bool focus) override;

    void setViewActiveFocus(qintptr viewId, bool value) override;
    bool activeFocus() const override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void hoverEnterEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void touchEvent(Qt::KeyboardModifiers qmods,
            const QList<QTouchEvent::TouchPoint> &qtTouchPoints,
            Qt::TouchPointStates qtTouchPointStates,
            ulong qtTimestamp) override;

    QString appId() const override;

    QCursor cursor() const override;

    void setKeymap(const QString &) override;
    QString keymap() const override;

    Mir::ShellChrome shellChrome() const override;

    SessionInterface* session() override { return m_session.data(); }

    bool inputAreaContains(const QPoint &) const override;

    void requestFocus() override;

    ////
    // Own API
    void setPosition(const QPoint newPosition);
    void updateState(Mir::State state);
    void setReady();
    miral::Window window() const { return m_window; }

    // useful for tests
    void setCloseTimer(AbstractTimer *timer);
    std::shared_ptr<SurfaceObserver> surfaceObserver() const;
    void setTextureProvider(CompositorTextureProvider *textureProvider);

public Q_SLOTS:
    ////
    // unity::shell::application::MirSurfaceInterface
    void requestState(Mir::State qmlState) override;

    ////
    // qtmir::MirSurfaceInterface
    void onCompositorSwappedBuffers() override;
    void setShellChrome(Mir::ShellChrome shellChrome) override;

private Q_SLOTS:
    void dropPendingBuffer();
    void onAttributeChanged(const MirWindowAttrib, const int);
    void onFramesPostedObserved();
    void emitSizeChanged();
    void setCursor(const QCursor &cursor);
    void onCloseTimedOut();
    void setInputBounds(const QRect &rect);

private:
    void syncSurfaceSizeWithItemSize();
    bool clientIsRunning() const;
    void updateExposure();
    void applyKeymap();
    void updateActiveFocus();
    void updateVisible();
    void onNameChanged(const QString &name);
    void onMinimumWidthChanged(int minWidth);
    void onMinimumHeightChanged(int minHeight);
    void onMaximumWidthChanged(int maxWidth);
    void onMaximumHeightChanged(int maxHeight);
    void onWidthIncrementChanged(int incWidth);
    void onHeightIncrementChanged(int incHeight);
    QPoint convertDisplayToLocalCoords(const QPoint &displayPos) const;
    QPoint convertLocalToDisplayCoords(const QPoint &localPos) const;

    bool updateTextureLocked(qintptr userId, CompositorTexture* compositorTexture);

    const miral::Window m_window;
    const std::shared_ptr<ExtraWindowInfo> m_extraInfo;
    QString m_name;
    MirWindowType m_type;
    int m_minWidth;
    int m_minHeight;
    int m_maxWidth;
    int m_maxHeight;
    int m_incWidth;
    int m_incHeight;

    const std::shared_ptr<mir::scene::Surface> m_surface; // keep copy of the Surface for lifecycle
    QPointer<SessionInterface> m_session;
    WindowControllerInterface *const m_controller;

    //FIXME -  have to save the state as Mir has no getter for it (bug:1357429)
    Mir::OrientationAngle m_orientationAngle;

    QTimer m_frameDropperTimer;

    mutable QMutex m_mutex;

    CompositorTextureProvider* m_textures;

    bool m_ready{false};
    bool m_visible;
    bool m_live;
    struct View {
        bool exposed;
    };
    QHash<qintptr, View> m_views;

    QSet<qintptr> m_activelyFocusedViews;
    bool m_neverSetSurfaceFocus{true};

    class SurfaceObserverImpl;
    std::shared_ptr<SurfaceObserverImpl> m_surfaceObserver;

    QPoint m_position;
    QPoint m_requestedPosition;
    QSize m_size;
    QSize m_pendingResize;
    QString m_keymap;

    QCursor m_cursor;
    Mir::State m_state; // FIXME: remove when Mir gains additional window states to match Mir::State
    Mir::ShellChrome m_shellChrome;

    QRect m_inputBounds;

    bool m_focused{false};

    enum ClosingState {
        NotClosing = 0,
        Closing = 1,
        CloseOverdue = 2
    };
    ClosingState m_closingState{NotClosing};
    AbstractTimer *m_closeTimer{nullptr};

    // assumes parent won't be destroyed before its children
    MirSurface *m_parentSurface;

    MirSurfaceListModel *m_childSurfaceList;
};

} // namespace qtmir

#endif // QTMIR_MIRSURFACE_H
