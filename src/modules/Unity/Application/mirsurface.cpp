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

#include "mirsurface.h"
#include "timestamp.h"

// from common dir
#include <debughelpers.h>

// mirserver
#include <surfaceobserver.h>

// Mir
#include <mir/geometry/rectangle.h>
#include <mir/events/event_builders.h>
#include <mir/shell/shell.h>
#include <mir_toolkit/event.h>

// mirserver
#include <logging.h>

using namespace qtmir;

#define DEBUG_MSG qCDebug(QTMIR_SURFACES).nospace() << "MirSurface[" << (void*)this << "," << appId() <<"]::" << __func__

namespace {

// Would be better if QMouseEvent had nativeModifiers
MirInputEventModifiers
getMirModifiersFromQt(Qt::KeyboardModifiers mods)
{
    MirInputEventModifiers m_mods = mir_input_event_modifier_none;
    if (mods & Qt::ShiftModifier)
        m_mods |= mir_input_event_modifier_shift;
    if (mods & Qt::ControlModifier)
        m_mods |= mir_input_event_modifier_ctrl;
    if (mods & Qt::AltModifier)
        m_mods |= mir_input_event_modifier_alt;
    if (mods & Qt::MetaModifier)
        m_mods |= mir_input_event_modifier_meta;

    return m_mods;
}

MirPointerButtons
getMirButtonsFromQt(Qt::MouseButtons buttons)
{
    MirPointerButtons result = 0;
    if (buttons & Qt::LeftButton)
        result |= mir_pointer_button_primary;
    if (buttons & Qt::RightButton)
        result |= mir_pointer_button_secondary;
    if (buttons & Qt::MiddleButton)
        result |= mir_pointer_button_tertiary;
    if (buttons & Qt::BackButton)
        result |= mir_pointer_button_back;
    if (buttons & Qt::ForwardButton)
        result |= mir_pointer_button_forward;

    return result;
}

mir::EventUPtr makeMirEvent(QMouseEvent *qtEvent, MirPointerAction action)
{
    auto timestamp = uncompressTimestamp<qtmir::Timestamp>(qtmir::Timestamp(qtEvent->timestamp()));
    auto modifiers = getMirModifiersFromQt(qtEvent->modifiers());
    auto buttons = getMirButtonsFromQt(qtEvent->buttons());

    return mir::events::make_event(0 /*DeviceID */, timestamp, std::vector<uint8_t>{} /* cookie */, modifiers, action,
                                   buttons, qtEvent->x(), qtEvent->y(), 0, 0, 0, 0);
}

mir::EventUPtr makeMirEvent(QHoverEvent *qtEvent, MirPointerAction action)
{
    auto timestamp = uncompressTimestamp<qtmir::Timestamp>(qtmir::Timestamp(qtEvent->timestamp()));

    MirPointerButtons buttons = 0;

    return mir::events::make_event(0 /*DeviceID */, timestamp, std::vector<uint8_t>{} /* cookie */, mir_input_event_modifier_none, action,
                                   buttons, qtEvent->posF().x(), qtEvent->posF().y(), 0, 0, 0, 0);
}

mir::EventUPtr makeMirEvent(QWheelEvent *qtEvent)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(qtEvent->timestamp()));
    auto modifiers = getMirModifiersFromQt(qtEvent->modifiers());
    auto buttons = getMirButtonsFromQt(qtEvent->buttons());

    return mir::events::make_event(0 /*DeviceID */, timestamp, std::vector<uint8_t>{} /* cookie */, modifiers, mir_pointer_action_motion,
                                   buttons, qtEvent->x(), qtEvent->y(),
                                   qtEvent->angleDelta().x(), qtEvent->angleDelta().y(),
                                   0, 0);
}

mir::EventUPtr makeMirEvent(QKeyEvent *qtEvent)
{
    MirKeyboardAction action = mir_keyboard_action_down;
    switch (qtEvent->type())
    {
    case QEvent::KeyPress:
        action = mir_keyboard_action_down;
        break;
    case QEvent::KeyRelease:
        action = mir_keyboard_action_up;
        break;
    default:
        break;
    }
    if (qtEvent->isAutoRepeat())
        action = mir_keyboard_action_repeat;

    return mir::events::make_event(0 /* DeviceID */, uncompressTimestamp<qtmir::Timestamp>(qtmir::Timestamp(qtEvent->timestamp())),
                           std::vector<uint8_t>{} /* cookie */, action, qtEvent->nativeVirtualKey(),
                           qtEvent->nativeScanCode(),
                           qtEvent->nativeModifiers());
}

mir::EventUPtr makeMirEvent(Qt::KeyboardModifiers qmods,
                            const QList<QTouchEvent::TouchPoint> &qtTouchPoints,
                            Qt::TouchPointStates /* qtTouchPointStates */,
                            ulong qtTimestamp)
{
    auto modifiers = getMirModifiersFromQt(qmods);
    auto ev = mir::events::make_event(0, uncompressTimestamp<qtmir::Timestamp>(qtmir::Timestamp(qtTimestamp)),
                                      std::vector<uint8_t>{} /* cookie */, modifiers);

    for (int i = 0; i < qtTouchPoints.count(); ++i) {
        auto touchPoint = qtTouchPoints.at(i);
        auto id = touchPoint.id();

        MirTouchAction action = mir_touch_action_change;
        if (touchPoint.state() == Qt::TouchPointReleased)
        {
            action = mir_touch_action_up;
        }
        if (touchPoint.state() == Qt::TouchPointPressed)
        {
            action = mir_touch_action_down;
        }

        MirTouchTooltype tooltype = mir_touch_tooltype_finger;
        if (touchPoint.flags() & QTouchEvent::TouchPoint::Pen)
            tooltype = mir_touch_tooltype_stylus;

        mir::events::add_touch(*ev, id, action, tooltype,
                               touchPoint.pos().x(), touchPoint.pos().y(),
                               touchPoint.pressure(),
                               touchPoint.rect().width(),
                               touchPoint.rect().height(),
                               0 /* size */);
    }

    return ev;
}

} // namespace {

MirSurface::MirSurface(std::shared_ptr<mir::scene::Surface> surface,
        SessionInterface* session,
        mir::shell::Shell* shell,
        std::shared_ptr<SurfaceObserver> observer)
    : MirSurfaceInterface()
    , m_surface(surface)
    , m_session(session)
    , m_shell(shell)
    , m_firstFrameDrawn(false)
    , m_orientationAngle(Mir::Angle0)
    , m_textureUpdated(false)
    , m_currentFrameNumber(0)
    , m_live(true)
{
    m_surfaceObserver = observer;
    if (observer) {
        connect(observer.get(), &SurfaceObserver::framesPosted, this, &MirSurface::onFramesPostedObserved);
        connect(observer.get(), &SurfaceObserver::attributeChanged, this, &MirSurface::onAttributeChanged);
        connect(observer.get(), &SurfaceObserver::nameChanged, this, &MirSurface::nameChanged);
        connect(observer.get(), &SurfaceObserver::cursorChanged, this, &MirSurface::setCursor);
        observer->setListener(this);
    }

    connect(session, &QObject::destroyed, this, &MirSurface::onSessionDestroyed);

    connect(&m_frameDropperTimer, &QTimer::timeout,
            this, &MirSurface::dropPendingBuffer);
    // Rationale behind the frame dropper and its interval value:
    //
    // We want to give ample room for Qt scene graph to have a chance to fetch and render
    // the next pending buffer before we take the drastic action of dropping it (so don't set
    // it anywhere close to our target render interval).
    //
    // We also want to guarantee a minimal frames-per-second (fps) frequency for client applications
    // as they get stuck on swap_buffers() if there's no free buffer to swap to yet (ie, they
    // are all pending consumption by the compositor, us). But on the other hand, we don't want
    // that minimal fps to be too high as that would mean this timer would be triggered way too often
    // for nothing causing unnecessary overhead as actually dropping frames from an app should
    // in practice rarely happen.
    m_frameDropperTimer.setInterval(200);
    m_frameDropperTimer.setSingleShot(false);
}

MirSurface::~MirSurface()
{
    qCDebug(QTMIR_SURFACES).nospace() << "MirSurface::~MirSurface this=" << this << " viewCount=" << m_views.count();

    Q_ASSERT(m_views.isEmpty());

    if (m_session) {
        m_session->removeSurface(this);
    }

    QMutexLocker locker(&m_mutex);
    m_surface->remove_observer(m_surfaceObserver);
}

void MirSurface::onFramesPostedObserved()
{
    if (!m_firstFrameDrawn) {
        m_firstFrameDrawn = true;
        Q_EMIT firstFrameDrawn();
    }

    // restart the frame dropper so that items have enough time to render the next frame.
    m_frameDropperTimer.start();

    Q_EMIT framesPosted();
}

void MirSurface::onAttributeChanged(const MirSurfaceAttrib attribute, const int /*value*/)
{
    switch (attribute) {
    case mir_surface_attrib_type:
        Q_EMIT typeChanged(type());
        break;
    case mir_surface_attrib_state:
        Q_EMIT stateChanged(state());
        break;
    case mir_surface_attrib_visibility:
        Q_EMIT visibleChanged(visible());
        break;
    default:
        break;
    }
}

Mir::Type MirSurface::type() const
{
    switch (m_surface->type()) {
    case mir_surface_type_normal:
        return Mir::NormalType;

    case mir_surface_type_utility:
        return Mir::UtilityType;

    case mir_surface_type_dialog:
        return Mir::DialogType;

    case mir_surface_type_gloss:
        return Mir::GlossType;

    case mir_surface_type_freestyle:
        return Mir::FreeStyleType;

    case mir_surface_type_menu:
        return Mir::MenuType;

    case mir_surface_type_inputmethod:
        return Mir::InputMethodType;

    case mir_surface_type_satellite:
        return Mir::SatelliteType;

    case mir_surface_type_tip:
        return Mir::TipType;

    default:
        return Mir::UnknownType;
    }
}

void MirSurface::dropPendingBuffer()
{
    QMutexLocker locker(&m_mutex);

    const void* const userId = (void*)123;  // TODO: Multimonitor support

    int framesPending = m_surface->buffers_ready_for_compositor(userId);
    if (framesPending > 0) {
        m_textureUpdated = false;

        locker.unlock();
        if (updateTexture()) {
            DEBUG_MSG << "() dropped=1 left=" << framesPending-1;
        } else {
            // If we haven't managed to update the texture, don't keep banging away.
            m_frameDropperTimer.stop();
            DEBUG_MSG << "() dropped=0" << " left=" << framesPending << " - failed to upate texture";
        }
        Q_EMIT frameDropped();
    } else {
        // The client can't possibly be blocked in swap buffers if the
        // queue is empty. So we can safely enter deep sleep now. If the
        // client provides any new frames, the timer will get restarted
        // via scheduleTextureUpdate()...
        m_frameDropperTimer.stop();
    }
}

void MirSurface::stopFrameDropper()
{
    DEBUG_MSG << "()";
    m_frameDropperTimer.stop();
}

void MirSurface::startFrameDropper()
{
    DEBUG_MSG << "()";
    if (!m_frameDropperTimer.isActive()) {
        m_frameDropperTimer.start();
    }
}

QSharedPointer<QSGTexture> MirSurface::texture()
{
    QMutexLocker locker(&m_mutex);

    if (!m_texture) {
        QSharedPointer<QSGTexture> texture(new MirBufferSGTexture);
        m_texture = texture.toWeakRef();
        return texture;
    } else {
        return m_texture.toStrongRef();
    }
}

bool MirSurface::updateTexture()
{
    QMutexLocker locker(&m_mutex);

    MirBufferSGTexture *texture = static_cast<MirBufferSGTexture*>(m_texture.data());
    if (!texture) return false;

    if (m_textureUpdated) {
        return texture->hasBuffer();
    }

    const void* const userId = (void*)123;
    auto renderables = m_surface->generate_renderables(userId);

    if (renderables.size() > 0 &&
            (m_surface->buffers_ready_for_compositor(userId) > 0 || !texture->hasBuffer())
        ) {
        // Avoid holding two buffers for the compositor at the same time. Thus free the current
        // before acquiring the next
        texture->freeBuffer();
        texture->setBuffer(renderables[0]->buffer());
        ++m_currentFrameNumber;

        if (texture->textureSize() != m_size) {
            m_size = texture->textureSize();
            QMetaObject::invokeMethod(this, "emitSizeChanged", Qt::QueuedConnection);
        }

        m_textureUpdated = true;
    }

    if (m_surface->buffers_ready_for_compositor(userId) > 0) {
        // restart the frame dropper to give MirSurfaceItems enough time to render the next frame.
        // queued since the timer lives in a different thread
        QMetaObject::invokeMethod(&m_frameDropperTimer, "start", Qt::QueuedConnection);
    }

    return texture->hasBuffer();
}

void MirSurface::onCompositorSwappedBuffers()
{
    QMutexLocker locker(&m_mutex);
    m_textureUpdated = false;
}

bool MirSurface::numBuffersReadyForCompositor()
{
    QMutexLocker locker(&m_mutex);
    const void* const userId = (void*)123;
    return m_surface->buffers_ready_for_compositor(userId);
}

void MirSurface::setFocus(bool focus)
{
    if (!m_session) {
        return;
    }

    // Temporary hotfix for http://pad.lv/1483752
    if (m_session->childSessions()->rowCount() > 0) {
        // has child trusted session, ignore any focus change attempts
        DEBUG_MSG << "(" << focus << ") - has child trusted session, ignore any focus change attempts";
        return;
    }

    DEBUG_MSG << "(" << focus << ")";

    if (focus) {
        m_shell->set_surface_attribute(m_session->session(), m_surface, mir_surface_attrib_focus, mir_surface_focused);
    } else {
        m_shell->set_surface_attribute(m_session->session(), m_surface, mir_surface_attrib_focus, mir_surface_unfocused);
    }
}

void MirSurface::close()
{
    if (m_surface) {
        m_surface->request_client_surface_close();
    }
}

void MirSurface::resize(int width, int height)
{
    int mirWidth = m_surface->size().width.as_int();
    int mirHeight = m_surface->size().height.as_int();

    bool mirSizeIsDifferent = width != mirWidth || height != mirHeight;

    if (clientIsRunning() && mirSizeIsDifferent) {
        mir::geometry::Size newMirSize(width, height);
        m_surface->resize(newMirSize);
        DEBUG_MSG << " old (" << mirWidth << "," << mirHeight << ")"
                  << ", new (" << width << "," << height << ")";
    }
}

QSize MirSurface::size() const
{
    return m_size;
}

Mir::State MirSurface::state() const
{
    switch (m_surface->state()) {
    case mir_surface_state_unknown:
        return Mir::UnknownState;
    case mir_surface_state_restored:
        return Mir::RestoredState;
    case mir_surface_state_minimized:
        return Mir::MinimizedState;
    case mir_surface_state_maximized:
        return Mir::MaximizedState;
    case mir_surface_state_vertmaximized:
        return Mir::VertMaximizedState;
    case mir_surface_state_fullscreen:
        return Mir::FullscreenState;
    case mir_surface_state_horizmaximized:
        return Mir::HorizMaximizedState;
    case mir_surface_state_hidden:
        return Mir::HiddenState;
    default:
        return Mir::UnknownState;
    }
}

Mir::OrientationAngle MirSurface::orientationAngle() const
{
    return m_orientationAngle;
}

void MirSurface::setOrientationAngle(Mir::OrientationAngle angle)
{
    MirOrientation mirOrientation;

    if (angle == m_orientationAngle) {
        return;
    }

    m_orientationAngle = angle;

    switch (angle) {
    case Mir::Angle0:
        mirOrientation = mir_orientation_normal;
        break;
    case Mir::Angle90:
        mirOrientation = mir_orientation_right;
        break;
    case Mir::Angle180:
        mirOrientation = mir_orientation_inverted;
        break;
    case Mir::Angle270:
        mirOrientation = mir_orientation_left;
        break;
    default:
        qCWarning(QTMIR_SURFACES, "Unsupported orientation angle: %d", angle);
        return;
    }

    if (m_surface) {
        m_surface->set_orientation(mirOrientation);
    }

    Q_EMIT orientationAngleChanged(angle);
}

QString MirSurface::name() const
{
    return QString::fromStdString(m_surface->name());
}

void MirSurface::setState(Mir::State qmlState)
{
    int mirState;

    switch (qmlState) {
    default:
    case Mir::UnknownState:
        mirState = mir_surface_state_unknown;
        break;

    case Mir::RestoredState:
        mirState = mir_surface_state_restored;
        break;

    case Mir::MinimizedState:
        mirState = mir_surface_state_minimized;
        break;

    case Mir::MaximizedState:
        mirState = mir_surface_state_maximized;
        break;

    case Mir::VertMaximizedState:
        mirState = mir_surface_state_vertmaximized;
        break;

    case Mir::FullscreenState:
        mirState = mir_surface_state_fullscreen;
        break;

    case Mir::HorizMaximizedState:
        mirState = mir_surface_state_horizmaximized;
        break;

    case Mir::HiddenState:
        mirState = mir_surface_state_hidden;
        break;
    }

    m_shell->set_surface_attribute(m_session->session(), m_surface, mir_surface_attrib_state, mirState);
}

void MirSurface::setLive(bool value)
{
    if (value != m_live) {
        m_live = value;
        Q_EMIT liveChanged(value);
    }
}

bool MirSurface::live() const
{
    return m_live;
}

bool MirSurface::visible() const
{
    return m_surface->query(mir_surface_attrib_visibility) == mir_surface_visibility_exposed;
}

void MirSurface::mousePressEvent(QMouseEvent *event)
{
    auto ev = makeMirEvent(event, mir_pointer_action_button_down);
    m_surface->consume(ev.get());
    event->accept();
}

void MirSurface::mouseMoveEvent(QMouseEvent *event)
{
    auto ev = makeMirEvent(event, mir_pointer_action_motion);
    m_surface->consume(ev.get());
    event->accept();
}

void MirSurface::mouseReleaseEvent(QMouseEvent *event)
{
    auto ev = makeMirEvent(event, mir_pointer_action_button_up);
    m_surface->consume(ev.get());
    event->accept();
}

void MirSurface::hoverEnterEvent(QHoverEvent *event)
{
    auto ev = makeMirEvent(event, mir_pointer_action_enter);
    m_surface->consume(ev.get());
    event->accept();
}

void MirSurface::hoverLeaveEvent(QHoverEvent *event)
{
    auto ev = makeMirEvent(event, mir_pointer_action_leave);
    m_surface->consume(ev.get());
    event->accept();
}

void MirSurface::hoverMoveEvent(QHoverEvent *event)
{
    auto ev = makeMirEvent(event, mir_pointer_action_motion);
    m_surface->consume(ev.get());
    event->accept();
}

void MirSurface::wheelEvent(QWheelEvent *event)
{
    auto ev = makeMirEvent(event);
    m_surface->consume(ev.get());
    event->accept();
}

void MirSurface::keyPressEvent(QKeyEvent *qtEvent)
{
    auto ev = makeMirEvent(qtEvent);
    m_surface->consume(ev.get());
    qtEvent->accept();
}

void MirSurface::keyReleaseEvent(QKeyEvent *qtEvent)
{
    auto ev = makeMirEvent(qtEvent);
    m_surface->consume(ev.get());
    qtEvent->accept();
}

void MirSurface::touchEvent(Qt::KeyboardModifiers mods,
                            const QList<QTouchEvent::TouchPoint> &touchPoints,
                            Qt::TouchPointStates touchPointStates,
                            ulong timestamp)
{
    auto ev = makeMirEvent(mods, touchPoints, touchPointStates, timestamp);
    m_surface->consume(ev.get());
}

bool MirSurface::clientIsRunning() const
{
    return (m_session &&
            (m_session->state() == Session::State::Running
             || m_session->state() == Session::State::Starting
             || m_session->state() == Session::State::Suspending))
        || !m_session;
}

bool MirSurface::isBeingDisplayed() const
{
    return !m_views.isEmpty();
}

void MirSurface::registerView(qintptr viewId)
{
    m_views.insert(viewId, MirSurface::View{false});
    DEBUG_MSG << "(" << viewId << ")" << " after=" << m_views.count();
    if (m_views.count() == 1) {
        Q_EMIT isBeingDisplayedChanged();
    }
}

void MirSurface::unregisterView(qintptr viewId)
{
    m_views.remove(viewId);
    DEBUG_MSG << "(" << viewId << ")" << " after=" << m_views.count() << " live=" << m_live;
    if (m_views.count() == 0) {
        Q_EMIT isBeingDisplayedChanged();
        if (m_session.isNull() || !m_live) {
            deleteLater();
        }
    }
    updateVisibility();
}

void MirSurface::setViewVisibility(qintptr viewId, bool visible)
{
    if (!m_views.contains(viewId)) return;

    m_views[viewId].visible = visible;
    updateVisibility();
}

void MirSurface::updateVisibility()
{
    // FIXME: https://bugs.launchpad.net/ubuntu/+source/unity8/+bug/1514556
    return;

    bool newVisible = false;
    QHashIterator<qintptr, View> i(m_views);
    while (i.hasNext()) {
        i.next();
        newVisible |= i.value().visible;
    }

    if (newVisible != visible()) {
        DEBUG_MSG << "(" << newVisible << ")";

        m_surface->configure(mir_surface_attrib_visibility,
                             newVisible ? mir_surface_visibility_exposed : mir_surface_visibility_occluded);
    }
}

unsigned int MirSurface::currentFrameNumber() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentFrameNumber;
}

void MirSurface::onSessionDestroyed()
{
    if (m_views.isEmpty()) {
        deleteLater();
    }
}

void MirSurface::emitSizeChanged()
{
    Q_EMIT sizeChanged(m_size);
}

QString MirSurface::appId() const
{
    QString appId;

    if (m_session && m_session->application()) {
        appId = m_session->application()->appId();
    } else {
        appId.append("-");
    }
    return appId;
}

QCursor MirSurface::cursor() const
{
    return m_cursor;
}

void MirSurface::setCursor(const QCursor &cursor)
{
    DEBUG_MSG << "(" << qtCursorShapeToStr(cursor.shape()) << ")";

    m_cursor = cursor;
    Q_EMIT cursorChanged(m_cursor);
}
