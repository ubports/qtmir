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
#include <QCursor>
#include <QMutex>
#include <QPointer>
#include <QSharedPointer>
#include <QSGTextureProvider>
#include <QTimer>
#include <QWeakPointer>
#include <QPair>

#include "mirbuffersgtexture.h"
#include "session.h"

// mirserver
#include "creationhints.h"

// mir
#include <mir/scene/surface.h>

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
            std::shared_ptr<SurfaceObserver> observer,
            const CreationHints &);
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

    bool visible() const override;

    Mir::OrientationAngle orientationAngle() const override;
    void setOrientationAngle(Mir::OrientationAngle angle) override;

    int minimumWidth() const override;
    int minimumHeight() const override;
    int maximumWidth() const override;
    int maximumHeight() const override;
    int widthIncrement() const override;
    int heightIncrement() const override;

    ////
    // qtmir::MirSurfaceInterface

    void setLive(bool value) override;

    bool isFirstFrameDrawn() const override { return m_firstFrameDrawn; }

    void stopFrameDropper() override;
    void startFrameDropper() override;

    bool isBeingDisplayed() const override;

    void registerView(qintptr viewId) override;
    void unregisterView(qintptr viewId) override;
    void setViewVisibility(qintptr viewId, bool visible) override;

    // methods called from the rendering (scene graph) thread:
    QSharedPointer<QSGTexture> texture() override;
    QSGTexture *weakTexture() const override { return m_texture.data(); }
    bool updateTexture() override;
    unsigned int currentFrameNumber() const override;
    bool numBuffersReadyForCompositor() override;
    // end of methods called from the rendering (scene graph) thread

    void setFocus(bool focus) override;

    void close() override;

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

    Mir::ShellChrome shellChrome() const override;

    QString keymapLayout() const override;
    QString keymapVariant() const override;
    Q_INVOKABLE void setKeymap(const QString &layout, const QString &variant) override;

public Q_SLOTS:
    void onCompositorSwappedBuffers() override;

    void setMinimumWidth(int) override;
    void setMinimumHeight(int) override;
    void setMaximumWidth(int) override;
    void setMaximumHeight(int) override;
    void setWidthIncrement(int) override;
    void setHeightIncrement(int) override;
    void setShellChrome(Mir::ShellChrome shellChrome) override;

private Q_SLOTS:
    void dropPendingBuffer();
    void onAttributeChanged(const MirSurfaceAttrib, const int);
    void onFramesPostedObserved();
    void onSessionDestroyed();
    void emitSizeChanged();
    void onKeymapChanged(const QString &layout, const QString &variant);
    void setCursor(const QCursor &cursor);

private:
    void syncSurfaceSizeWithItemSize();
    bool clientIsRunning() const;
    void updateVisibility();

    std::shared_ptr<mir::scene::Surface> m_surface;
    QPointer<SessionInterface> m_session;
    mir::shell::Shell *const m_shell;
    bool m_firstFrameDrawn;

    //FIXME -  have to save the state as Mir has no getter for it (bug:1357429)
    Mir::OrientationAngle m_orientationAngle;

    QTimer m_frameDropperTimer;

    mutable QMutex m_mutex;

    // Lives in the rendering (scene graph) thread
    QWeakPointer<QSGTexture> m_texture;
    bool m_textureUpdated;
    unsigned int m_currentFrameNumber;

    bool m_live;
    struct View {
        bool visible;
    };
    QHash<qintptr, View> m_views;

    std::shared_ptr<SurfaceObserver> m_surfaceObserver;

    QSize m_size;
    QPair<QString,QString> m_keyMap; // pair of layout+variant

    QCursor m_cursor;
    Mir::ShellChrome m_shellChrome;

    int m_minimumWidth{0};
    int m_minimumHeight{0};
    int m_maximumWidth{0};
    int m_maximumHeight{0};
    int m_widthIncrement{0};
    int m_heightIncrement{0};
};

} // namespace qtmir

#endif // QTMIR_MIRSURFACE_H
