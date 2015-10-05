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

#ifndef QTMIR_MIRSURFACEINTERFACE_H
#define QTMIR_MIRSURFACEINTERFACE_H

// Unity API
#include <unity/shell/application/MirSurfaceInterface.h>

#include "session_interface.h"

// Qt
#include <QSharedPointer>
#include <QTouchEvent>

class QHoverEvent;
class QMouseEvent;
class QKeyEvent;
class QSGTexture;

namespace qtmir {

class MirSurfaceInterface : public unity::shell::application::MirSurfaceInterface
{
    Q_OBJECT
public:
    MirSurfaceInterface(QObject *parent = nullptr) : unity::shell::application::MirSurfaceInterface(parent) {}
    virtual ~MirSurfaceInterface() {}

    virtual void setLive(bool value) = 0;

    virtual void setVisible(bool visible) = 0;

    virtual bool isFirstFrameDrawn() const = 0;

    virtual void stopFrameDropper() = 0;
    virtual void startFrameDropper() = 0;

    virtual bool isBeingDisplayed() const = 0;
    virtual void incrementViewCount() = 0;
    virtual void decrementViewCount() = 0;

    // methods called from the rendering (scene graph) thread:
    virtual QSharedPointer<QSGTexture> texture() = 0;
    virtual QSGTexture *weakTexture() const = 0;
    virtual void updateTexture() = 0;
    virtual unsigned int currentFrameNumber() const = 0;
    virtual bool numBuffersReadyForCompositor() = 0;
    // end of methods called from the rendering (scene graph) thread

    virtual void setFocus(bool focus) = 0;

    virtual void mousePressEvent(QMouseEvent *event) = 0;
    virtual void mouseMoveEvent(QMouseEvent *event) = 0;
    virtual void mouseReleaseEvent(QMouseEvent *event) = 0;
    virtual void hoverEnterEvent(QHoverEvent *event) = 0;
    virtual void hoverLeaveEvent(QHoverEvent *event) = 0;
    virtual void hoverMoveEvent(QHoverEvent *event) = 0;

    virtual void keyPressEvent(QKeyEvent *event) = 0;
    virtual void keyReleaseEvent(QKeyEvent *event) = 0;

    virtual void touchEvent(Qt::KeyboardModifiers qmods,
            const QList<QTouchEvent::TouchPoint> &qtTouchPoints,
            Qt::TouchPointStates qtTouchPointStates,
            ulong qtTimestamp) = 0;

    virtual QString appId() const = 0;

public Q_SLOTS:
    virtual void onCompositorSwappedBuffers() = 0;

Q_SIGNALS:
    void firstFrameDrawn();
    void framesPosted();
    void isBeingDisplayedChanged();
};

} // namespace qtmir

#endif // QTMIR_MIRSURFACEINTERFACE_H
