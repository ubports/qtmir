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

#ifndef QTMIR_MIRSURFACE_H
#define QTMIR_MIRSURFACE_H

#include "mirsurfaceinterface.h"

// Qt
#include <QMutex>
#include <QPointer>
#include <QSharedPointer>
#include <QSGTextureProvider>
#include <QTimer>
#include <QWeakPointer>

#include "mirbuffersgtexture.h"
#include "session.h"

// mir
#include <mir/scene/surface.h>
#include <mir_toolkit/common.h>

namespace mir { namespace shell { class Shell; }}

class SurfaceObserver;

namespace qtmir {

class MirSurface : public MirSurfaceInterface
{
    Q_OBJECT

public:
    MirSurface(std::shared_ptr<mir::scene::Surface> surface,
            SessionInterface* session,
            mir::shell::Shell *shell,
            std::shared_ptr<SurfaceObserver> observer);
    virtual ~MirSurface();

    ////
    // unity::shell::application::MirSurfaceInterface

    Mir::Type type() const override;

    QString name() const override;

    QSize size() const override;
    void resize(int width, int height) override;
    void resize(const QSize &size) override { resize(size.width(), size.height()); }

    Mir::State state() const override;
    void setState(Mir::State qmlState) override;

    bool live() const override;

    Mir::OrientationAngle orientationAngle() const override;
    void setOrientationAngle(Mir::OrientationAngle angle) override;

    ////
    // qtmir::MirSurfaceInterface

    void setLive(bool value) override;

    bool isFirstFrameDrawn() const override { return m_firstFrameDrawn; }

    void stopFrameDropper() override;
    void startFrameDropper() override;

    bool isBeingDisplayed() const override;
    void incrementViewCount() override;
    void decrementViewCount() override;

    // methods called from the rendering (scene graph) thread:
    QSharedPointer<QSGTexture> texture() override;
    void updateTexture() override;
    unsigned int currentFrameNumber() const override;
    bool numBuffersReadyForCompositor() override;
    // end of methods called from the rendering (scene graph) thread

    void setFocus(bool focus) override;

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

public Q_SLOTS:
    void onCompositorSwappedBuffers() override;

private Q_SLOTS:
    void dropPendingBuffer();
    void onAttributeChanged(const MirSurfaceAttrib, const int);
    void onFramesPostedObserved();
    void onSessionDestroyed();
    void emitSizeChanged();

private:
    void syncSurfaceSizeWithItemSize();
    bool clientIsRunning() const;

    std::shared_ptr<mir::scene::Surface> m_surface;
    QPointer<SessionInterface> m_session;
    mir::shell::Shell *const m_shell;
    bool m_firstFrameDrawn;

    //FIXME -  have to save the state as Mir has no getter for it (bug:1357429)
    Mir::OrientationAngle m_orientationAngle;

    QTimer m_frameDropperTimer;

    QMutex m_mutex;

    // Lives in the rendering (scene graph) thread
    QWeakPointer<QSGTexture> m_texture;
    bool m_textureUpdated;
    unsigned int m_currentFrameNumber;

    bool m_live;
    int m_viewCount;

    std::shared_ptr<SurfaceObserver> m_surfaceObserver;

    QSize m_size;
};

} // namespace qtmir

#endif // QTMIR_MIRSURFACE_H
