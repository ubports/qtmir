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

mir::EventUPtr makeMirEvent(QMouseEvent *qtEvent, MirPointerAction action)
{
    auto timestamp = std::chrono::milliseconds(qtEvent->timestamp());
    auto modifiers = getMirModifiersFromQt(qtEvent->modifiers());

    MirPointerButtons buttons = 0;
    if (qtEvent->buttons() & Qt::LeftButton)
        buttons |= mir_pointer_button_primary;
    if (qtEvent->buttons() & Qt::RightButton)
        buttons |= mir_pointer_button_secondary;
    if (qtEvent->buttons() & Qt::MidButton)
        buttons |= mir_pointer_button_tertiary;

    return mir::events::make_event(0 /*DeviceID */, timestamp, 0 /* mac */, modifiers, action,
                                   buttons, qtEvent->x(), qtEvent->y(), 0, 0, 0, 0);
}

mir::EventUPtr makeMirEvent(QHoverEvent *qtEvent, MirPointerAction action)
{
    auto timestamp = std::chrono::milliseconds(qtEvent->timestamp());

    MirPointerButtons buttons = 0;

    return mir::events::make_event(0 /*DeviceID */, timestamp, 0 /* mac */, mir_input_event_modifier_none, action,
                                   buttons, qtEvent->posF().x(), qtEvent->posF().y(), 0, 0, 0, 0);
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

    return mir::events::make_event(0 /* DeviceID */, std::chrono::milliseconds(qtEvent->timestamp()),
                           0 /* mac */, action, qtEvent->nativeVirtualKey(),
                           qtEvent->nativeScanCode(),
                           qtEvent->nativeModifiers());
}

mir::EventUPtr makeMirEvent(Qt::KeyboardModifiers qmods,
                            const QList<QTouchEvent::TouchPoint> &qtTouchPoints,
                            Qt::TouchPointStates /* qtTouchPointStates */,
                            ulong qtTimestamp)
{
    auto modifiers = getMirModifiersFromQt(qmods);
    auto ev = mir::events::make_event(0, std::chrono::milliseconds(qtTimestamp),
                                      0 /* mac */, modifiers);

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
    , m_viewCount(0)
{
    m_surfaceObserver = observer;
    if (observer) {
        connect(observer.get(), &SurfaceObserver::framesPosted, this, &MirSurface::onFramesPostedObserved);
        connect(observer.get(), &SurfaceObserver::attributeChanged, this, &MirSurface::onAttributeChanged);
        connect(observer.get(), &SurfaceObserver::nameChanged, this, &MirSurface::onNameChanged);
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

    if (m_surface) {
        m_name = QString::fromStdString(m_surface->name());
    }
}

MirSurface::~MirSurface()
{
    qCDebug(QTMIR_SURFACES).nospace() << "MirSurface::~MirSurface this=" << this << " viewCount=" << m_viewCount;

    Q_ASSERT(m_viewCount == 0);

    if (m_session) {
        m_session->setSurface(nullptr);
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

void MirSurface::onNameChanged(const QString &name)
{
    if (name != m_name) {
        m_name = name;
        Q_EMIT nameChanged(m_name);
    }
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
        // The line below looks like an innocent, effect-less, getter. But as this
        // method returns a unique_pointer, not holding its reference causes the
        // buffer to be destroyed/released straight away.
        for (auto const & item : m_surface->generate_renderables(userId))
            item->buffer();
        qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::dropPendingBuffer()"
            << "surface =" << this
            << "buffer dropped."
            << framesPending-1
            << "left.";
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
    qCDebug(QTMIR_SURFACES) << "MirSurface::stopFrameDropper surface = " << this;
    m_frameDropperTimer.stop();
}

void MirSurface::startFrameDropper()
{
    qCDebug(QTMIR_SURFACES) << "MirSurface::startFrameDropper surface = " << this;
    if (!m_frameDropperTimer.isActive()) {
        m_frameDropperTimer.start();
    }
}

QSharedPointer<QSGTexture> MirSurface::texture()
{
    if (!m_texture) {
        QSharedPointer<QSGTexture> texture(new MirBufferSGTexture);
        m_texture = texture.toWeakRef();
        return texture;
    } else {
        return m_texture.toStrongRef();
    }
}

void MirSurface::updateTexture()
{
    QMutexLocker locker(&m_mutex);

    if (m_textureUpdated) {
        return;
    }

    Q_ASSERT(!m_texture.isNull());
    MirBufferSGTexture *texture = static_cast<MirBufferSGTexture*>(m_texture.data());

    const void* const userId = (void*)123;
    auto renderables = m_surface->generate_renderables(userId);

    if (m_surface->buffers_ready_for_compositor(userId) > 0 && renderables.size() > 0) {
        // Avoid holding two buffers for the compositor at the same time. Thus free the current
        // before acquiring the next
        texture->freeBuffer();
        texture->setBuffer(renderables[0]->buffer());
        ++m_currentFrameNumber;

        if (texture->textureSize() != m_size) {
            m_size = texture->textureSize();
            QMetaObject::invokeMethod(this, "emitSizeChanged", Qt::QueuedConnection);
        }
    }

    if (m_surface->buffers_ready_for_compositor(userId) > 0) {
        // restart the frame dropper to give MirSurfaceItems enough time to render the next frame.
        // queued since the timer lives in a different thread
        QMetaObject::invokeMethod(&m_frameDropperTimer, "start", Qt::QueuedConnection);
    }

    m_textureUpdated = true;
}

void MirSurface::onCompositorSwappedBuffers()
{
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
        qCDebug(QTMIR_SURFACES).nospace() << "MirSurface[" << appId() << "]::setFocus(" << focus
            << ") - has child trusted session, ignore any focus change attempts";
        return;
    }

    qCDebug(QTMIR_SURFACES).nospace() << "MirSurface[" << appId() << "]::setFocus(" << focus << ")";

    if (focus) {
        m_shell->set_surface_attribute(m_session->session(), m_surface, mir_surface_attrib_focus, mir_surface_focused);
    } else {
        m_shell->set_surface_attribute(m_session->session(), m_surface, mir_surface_attrib_focus, mir_surface_unfocused);
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
        qCDebug(QTMIR_SURFACES) << "MirSurface::resize"
                << "surface =" << this
                << ", old (" << mirWidth << "," << mirHeight << ")"
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
    return m_name;
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

void MirSurface::mousePressEvent(QMouseEvent *event)
{
    auto ev = makeMirEvent(event, mir_pointer_action_button_down);
    m_surface->consume(*ev);
    event->accept();
}

void MirSurface::mouseMoveEvent(QMouseEvent *event)
{
    auto ev = makeMirEvent(event, mir_pointer_action_motion);
    m_surface->consume(*ev);
    event->accept();
}

void MirSurface::mouseReleaseEvent(QMouseEvent *event)
{
    auto ev = makeMirEvent(event, mir_pointer_action_button_up);
    m_surface->consume(*ev);
    event->accept();
}

void MirSurface::hoverEnterEvent(QHoverEvent *event)
{
    auto ev = makeMirEvent(event, mir_pointer_action_enter);
    m_surface->consume(*ev);
    event->accept();
}

void MirSurface::hoverLeaveEvent(QHoverEvent *event)
{
    auto ev = makeMirEvent(event, mir_pointer_action_leave);
    m_surface->consume(*ev);
    event->accept();
}

void MirSurface::hoverMoveEvent(QHoverEvent *event)
{
    auto ev = makeMirEvent(event, mir_pointer_action_motion);
    m_surface->consume(*ev);
    event->accept();
}

void MirSurface::keyPressEvent(QKeyEvent *qtEvent)
{
    auto ev = makeMirEvent(qtEvent);
    m_surface->consume(*ev);
    qtEvent->accept();
}

void MirSurface::keyReleaseEvent(QKeyEvent *qtEvent)
{
    auto ev = makeMirEvent(qtEvent);
    m_surface->consume(*ev);
    qtEvent->accept();
}

void MirSurface::touchEvent(Qt::KeyboardModifiers mods,
                            const QList<QTouchEvent::TouchPoint> &touchPoints,
                            Qt::TouchPointStates touchPointStates,
                            ulong timestamp)
{
    auto ev = makeMirEvent(mods, touchPoints, touchPointStates, timestamp);
    m_surface->consume(*ev);
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
    return m_viewCount > 0;
}

void MirSurface::incrementViewCount()
{
    ++m_viewCount;
    qCDebug(QTMIR_SURFACES).nospace() << "MirSurface::incrementViewCount after=" << m_viewCount;
    if (m_viewCount == 1) {
        Q_EMIT isBeingDisplayedChanged();
    }
}

void MirSurface::decrementViewCount()
{
    Q_ASSERT(m_viewCount > 0);
    --m_viewCount;
    qCDebug(QTMIR_SURFACES).nospace() << "MirSurface::decrementViewCount after=" << m_viewCount;
    if (m_viewCount == 0) {
        Q_EMIT isBeingDisplayedChanged();
        if (m_session.isNull() || !m_live) {
            deleteLater();
        }
    }
}

unsigned int MirSurface::currentFrameNumber() const
{
    return m_currentFrameNumber;
}

void MirSurface::onSessionDestroyed()
{
    if (m_viewCount == 0) {
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
