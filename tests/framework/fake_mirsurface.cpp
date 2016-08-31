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

#include "fake_mirsurface.h"

namespace qtmir
{

FakeMirSurface::TouchEvent::TouchEvent(Qt::KeyboardModifiers mods,
                                       const QList<QTouchEvent::TouchPoint> &points,
                                       Qt::TouchPointStates states,
                                       ulong timestamp)
    : keyboardModifiers(mods)
    , touchPoints(points)
    , states(states)
    , timestamp(timestamp)
{
}

FakeMirSurface::TouchEvent::~TouchEvent()
{
}

FakeMirSurface::FakeMirSurface(QObject *parent)
    : MirSurfaceInterface(parent)
    , m_isFirstFrameDrawn(false)
    , m_isFrameDropperRunning(true)
    , m_live(true)
    , m_state(Mir::RestoredState)
    , m_orientationAngle(Mir::Angle0)
    , m_visible(true)
    , m_focused(false)
{
}

FakeMirSurface::~FakeMirSurface()
{
}

Mir::Type FakeMirSurface::type() const { return Mir::NormalType; }

QString FakeMirSurface::name() const { return QString("Fake MirSurface"); }

QString FakeMirSurface::persistentId() const { return QString("FakeSurfaceId"); }

QSize FakeMirSurface::size() const { return m_size; }

void FakeMirSurface::resize(int width, int height)
{
    if (m_size.width() != width || m_size.height() != height) {
        m_size.setWidth(width);
        m_size.setHeight(height);
        Q_EMIT sizeChanged(m_size);
    }
}

void FakeMirSurface::resize(const QSize &size) { resize(size.width(), size.height()); }

Mir::State FakeMirSurface::state() const { return m_state; }

void FakeMirSurface::setState(Mir::State qmlState)
{
    if (qmlState != m_state) {
        m_state = qmlState;
        Q_EMIT stateChanged(m_state);
    }
}

bool FakeMirSurface::live() const { return m_live; }

bool FakeMirSurface::visible() const { return m_visible; }

Mir::OrientationAngle FakeMirSurface::orientationAngle() const { return m_orientationAngle; }

void FakeMirSurface::setOrientationAngle(Mir::OrientationAngle angle)
{
    if (m_orientationAngle != angle) {
        m_orientationAngle = angle;
        Q_EMIT orientationAngleChanged(m_orientationAngle);
    }
}

bool FakeMirSurface::isFirstFrameDrawn() const
{
    return m_isFirstFrameDrawn;
}

void FakeMirSurface::stopFrameDropper()
{
    m_isFrameDropperRunning = false;
}

void FakeMirSurface::startFrameDropper()
{
    m_isFrameDropperRunning = true;
}

void FakeMirSurface::setLive(bool value)
{
    if (m_live != value) {
        m_live = value;
        Q_EMIT liveChanged(m_live);
    }
}

void FakeMirSurface::setViewVisibility(qintptr viewId, bool visible) {
    if (!m_views.contains(viewId)) return;

    m_views[viewId] = visible;
    updateVisibility();
}

bool FakeMirSurface::isBeingDisplayed() const { return !m_views.isEmpty(); }

void FakeMirSurface::registerView(qintptr viewId)
{
    m_views.insert(viewId, false);
    if (m_views.count() == 1) {
        Q_EMIT isBeingDisplayedChanged();
    }
}

void FakeMirSurface::unregisterView(qintptr viewId)
{
    m_views.remove(viewId);
    if (m_views.count() == 0) {
        Q_EMIT isBeingDisplayedChanged();
    }
    updateVisibility();
}

QSharedPointer<QSGTexture> FakeMirSurface::texture() { return QSharedPointer<QSGTexture>(); }

QSGTexture *FakeMirSurface::weakTexture() const { return nullptr; }

bool FakeMirSurface::updateTexture() { return true; }

unsigned int FakeMirSurface::currentFrameNumber() const { return 0; }

bool FakeMirSurface::numBuffersReadyForCompositor() { return 0; }

void FakeMirSurface::setFocused(bool focus) { m_focused = focus; }

void FakeMirSurface::mousePressEvent(QMouseEvent *) {}

void FakeMirSurface::mouseMoveEvent(QMouseEvent *) {}

void FakeMirSurface::mouseReleaseEvent(QMouseEvent *) {}

void FakeMirSurface::hoverEnterEvent(QHoverEvent *) {}

void FakeMirSurface::hoverLeaveEvent(QHoverEvent *) {}

void FakeMirSurface::hoverMoveEvent(QHoverEvent *) {}

void FakeMirSurface::wheelEvent(QWheelEvent *) {}

void FakeMirSurface::keyPressEvent(QKeyEvent *) {}

void FakeMirSurface::keyReleaseEvent(QKeyEvent *) {}

void FakeMirSurface::touchEvent(Qt::KeyboardModifiers mods,
                                const QList<QTouchEvent::TouchPoint> &points,
                                Qt::TouchPointStates states,
                                ulong timestamp)
{
    m_touchesReceived.append(TouchEvent(mods, points, states, timestamp));
}

QString FakeMirSurface::appId() const { return "foo-app"; }

void FakeMirSurface::onCompositorSwappedBuffers() {}

void FakeMirSurface::drawFirstFrame()
{
    if (!m_isFirstFrameDrawn) {
        m_isFirstFrameDrawn = true;
        Q_EMIT firstFrameDrawn();
    }
}

bool FakeMirSurface::isFrameDropperRunning() const { return m_isFrameDropperRunning; }

QList<FakeMirSurface::TouchEvent> &FakeMirSurface::touchesReceived() { return m_touchesReceived; }

void FakeMirSurface::updateVisibility()
{
    bool newVisible = false;
    QHashIterator<int, bool> i(m_views);
    while (i.hasNext()) {
        i.next();
        newVisible |= i.value();
    }

    if (m_visible != newVisible) {
        m_visible = newVisible;
        Q_EMIT visibleChanged(newVisible);
    }
}

void FakeMirSurface::setSession(SessionInterface *session)
{
    m_session = session;
}

} // namespace qtmir
