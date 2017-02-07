/*
 * Copyright (C) 2015-2017 Canonical, Ltd.
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
#include "mirsurfacelistmodel.h"
#include "namedcursor.h"
#include "session_interface.h"
#include "timer.h"
#include "timestamp.h"

// from common dir
#include <debughelpers.h>
#include "mirqtconversion.h"

// mirserver
#include <eventbuilder.h>
#include <surfaceobserver.h>
#include "screen.h"

// Mir
#include <mir/geometry/rectangle.h>
#include <mir/scene/surface.h>
#include <mir/scene/surface_observer.h>
#include <mir/version.h>

// mirserver
#include <logging.h>

// Qt
#include <QQmlEngine>
#include <QScreen>

// std
#include <limits>

using namespace qtmir;
namespace unityapi = unity::shell::application;

#define DEBUG_MSG qCDebug(QTMIR_SURFACES).nospace() << "MirSurface[" << (void*)this << "," << appId() << "]::" << __func__
#define INFO_MSG qCInfo(QTMIR_SURFACES).nospace() << "MirSurface[" << (void*)this << "," << appId() << "]::" << __func__
#define WARNING_MSG qCWarning(QTMIR_SURFACES).nospace() << "MirSurface[" << (void*)this << "," << appId() << "]::" << __func__

namespace {

enum class DirtyState {
    Clean = 0,
    Name = 1 << 1,
    Type = 1 << 2,
    State = 1 << 3,
    RestoreRect = 1 << 4,
    Children = 1 << 5,
    MinSize = 1 << 6,
    MaxSize = 1 << 7,
};
Q_DECLARE_FLAGS(DirtyStates, DirtyState)

} // namespace {

class MirSurface::SurfaceObserverImpl : public SurfaceObserver, public mir::scene::SurfaceObserver
{
public:
    SurfaceObserverImpl();
    virtual ~SurfaceObserverImpl();

    void setListener(QObject *listener);

    void attrib_changed(MirWindowAttrib, int) override;
    void resized_to(mir::geometry::Size const&) override;
    void moved_to(mir::geometry::Point const&) override {}
    void hidden_set_to(bool) override {}

    // Get new frame notifications from Mir, called from a Mir thread.
    void frame_posted(int frames_available, mir::geometry::Size const& size ) override;

    void alpha_set_to(float) override {}
    void transformation_set_to(glm::mat4 const&) override {}
    void reception_mode_set_to(mir::input::InputReceptionMode) override {}
    void cursor_image_set_to(mir::graphics::CursorImage const&) override;
    void orientation_set_to(MirOrientation) override {}
    void client_surface_close_requested() override {}
    void keymap_changed(MirInputDeviceId, std::string const&, std::string const&,
                        std::string const&, std::string const&) override {}
    void renamed(char const * name) override;
    void cursor_image_removed() override;

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 25, 0)
    void placed_relative(mir::geometry::Rectangle const& placement) override;
#endif

private:
    QCursor createQCursorFromMirCursorImage(const mir::graphics::CursorImage &cursorImage);
    QObject *m_listener;
    bool m_framesPosted;
    QMap<QByteArray, Qt::CursorShape> m_cursorNameToShape;
};


MirSurface::MirSurface(NewWindow newWindowInfo,
        WindowControllerInterface* controller,
        SessionInterface *session,
        MirSurface *parentSurface)
    : MirSurfaceInterface()
    , m_window{newWindowInfo.windowInfo.window()}
    , m_extraInfo{getExtraInfo(newWindowInfo.windowInfo)}
    , m_name{QString::fromStdString(newWindowInfo.windowInfo.name())}
    , m_type{newWindowInfo.windowInfo.type()}
    , m_minWidth{newWindowInfo.windowInfo.min_width().as_int()}
    , m_minHeight{newWindowInfo.windowInfo.min_height().as_int()}
    , m_maxWidth{newWindowInfo.windowInfo.max_width().as_int()}
    , m_maxHeight{newWindowInfo.windowInfo.max_height().as_int()}
    , m_incWidth{newWindowInfo.windowInfo.width_inc().as_int()}
    , m_incHeight{newWindowInfo.windowInfo.height_inc().as_int()}
    , m_surface(newWindowInfo.surface)
    , m_session(session)
    , m_controller(controller)
    , m_orientationAngle(Mir::Angle0)
    , m_textureUpdated(false)
    , m_currentFrameNumber(0)
    , m_visible(newWindowInfo.windowInfo.is_visible())
    , m_live(true)
    , m_surfaceObserver(std::make_shared<SurfaceObserverImpl>())
    , m_size(toQSize(m_window.size()))
    , m_state(toQtState(newWindowInfo.windowInfo.state()))
    , m_shellChrome(Mir::NormalChrome)
    , m_parentSurface(parentSurface)
    , m_childSurfaceList(new MirSurfaceListModel(this))
{
    INFO_MSG << "("
        << "type=" << mirSurfaceTypeToStr(m_type)
        << ",state=" << unityapiMirStateToStr(m_state)
        << ",size=(" << m_size.width() << "," << m_size.height() << ")"
        << ",parentSurface=" << m_parentSurface
        << ")";

    m_position = convertDisplayToLocalCoords(toQPoint(m_window.top_left()));

    SurfaceObserver::registerObserverForSurface(m_surfaceObserver.get(), m_surface.get());
    m_surface->add_observer(m_surfaceObserver);

    //m_shellChrome = creationHints.shellChrome; TODO - where will this come from now?

    connect(m_surfaceObserver.get(), &SurfaceObserver::framesPosted, this, &MirSurface::onFramesPostedObserved);
    connect(m_surfaceObserver.get(), &SurfaceObserver::attributeChanged, this, &MirSurface::onAttributeChanged);
    connect(m_surfaceObserver.get(), &SurfaceObserver::nameChanged, this, &MirSurface::onNameChanged);
    connect(m_surfaceObserver.get(), &SurfaceObserver::cursorChanged, this, &MirSurface::setCursor);
    connect(m_surfaceObserver.get(), &SurfaceObserver::minimumWidthChanged, this, &MirSurface::onMinimumWidthChanged);
    connect(m_surfaceObserver.get(), &SurfaceObserver::minimumHeightChanged, this, &MirSurface::onMinimumHeightChanged);
    connect(m_surfaceObserver.get(), &SurfaceObserver::maximumWidthChanged, this, &MirSurface::onMaximumWidthChanged);
    connect(m_surfaceObserver.get(), &SurfaceObserver::maximumHeightChanged, this, &MirSurface::onMaximumHeightChanged);
    connect(m_surfaceObserver.get(), &SurfaceObserver::widthIncrementChanged, this, &MirSurface::onWidthIncrementChanged);
    connect(m_surfaceObserver.get(), &SurfaceObserver::heightIncrementChanged, this, &MirSurface::onHeightIncrementChanged);
    connect(m_surfaceObserver.get(), &SurfaceObserver::shellChromeChanged, this, [&](MirShellChrome shell_chrome) {
        setShellChrome(static_cast<Mir::ShellChrome>(shell_chrome));
    });
    connect(m_surfaceObserver.get(), &SurfaceObserver::inputBoundsChanged, this, &MirSurface::setInputBounds);
    connect(m_surfaceObserver.get(), &SurfaceObserver::confinesMousePointerChanged, this, &MirSurface::confinesMousePointerChanged);
    m_surfaceObserver->setListener(this);

    //connect(session, &QObject::destroyed, this, &MirSurface::onSessionDestroyed); // TODO try using Shared pointer for lifecycle
    connect(session, &SessionInterface::stateChanged, this, [this]() {
        if (clientIsRunning() && m_pendingResize.isValid()) {
            resize(m_pendingResize.width(), m_pendingResize.height());
            m_pendingResize = QSize(-1, -1);
        }
    });

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

    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    setCloseTimer(new Timer);

    m_requestedPosition.rx() = std::numeric_limits<int>::min();
    m_requestedPosition.ry() = std::numeric_limits<int>::min();
}

MirSurface::~MirSurface()
{
    INFO_MSG << "() viewCount=" << m_views.count();

    Q_ASSERT(m_views.isEmpty());

    QMutexLocker locker(&m_mutex);
    m_surface->remove_observer(m_surfaceObserver);

    delete m_closeTimer;

    Q_EMIT destroyed(this); // Early warning, while MirSurface methods can still be accessed.
}

void MirSurface::onFramesPostedObserved()
{
    // restart the frame dropper so that items have enough time to render the next frame.
    m_frameDropperTimer.start();

    Q_EMIT framesPosted();
}

void MirSurface::onAttributeChanged(const MirWindowAttrib attribute, const int /*value*/)
{
    switch (attribute) {
    case mir_window_attrib_type:
        DEBUG_MSG << " type = " << mirSurfaceTypeToStr(type());
        Q_EMIT typeChanged(type());
        break;
    default:
        break;
    }
}

Mir::Type MirSurface::type() const
{
    switch (m_type) {
    case mir_window_type_normal:
        return Mir::NormalType;

    case mir_window_type_utility:
        return Mir::UtilityType;

    case mir_window_type_dialog:
        return Mir::DialogType;

    case mir_window_type_gloss:
        return Mir::GlossType;

    case mir_window_type_freestyle:
        return Mir::FreeStyleType;

    case mir_window_type_menu:
        return Mir::MenuType;

    case mir_window_type_inputmethod:
        return Mir::InputMethodType;

    case mir_window_type_satellite:
        return Mir::SatelliteType;

    case mir_window_type_tip:
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
    if (framesPending == 0) {
        // The client can't possibly be blocked in swap buffers if the
        // queue is empty. So we can safely enter deep sleep now. If the
        // client provides any new frames, the timer will get restarted
        // via scheduleTextureUpdate()...
        m_frameDropperTimer.stop();
        return;
    }

    m_textureUpdated = false;
    auto texture = static_cast<MirBufferSGTexture*>(m_texture.data());

    auto renderables = m_surface->generate_renderables(userId);
    if (renderables.size() > 0) {
        ++m_currentFrameNumber;
        if (texture) {
            texture->freeBuffer();
            texture->setBuffer(renderables[0]->buffer());
            if (texture->textureSize() != size()) {
                m_size = texture->textureSize();
                QMetaObject::invokeMethod(this, "emitSizeChanged", Qt::QueuedConnection);
            }
            m_textureUpdated = true;

            framesPending = m_surface->buffers_ready_for_compositor(userId);
            if (framesPending > 0) {
                // restart the frame dropper to give MirSurfaceItems enough time to render the next frame.
                // queued since the timer lives in a different thread
                DEBUG_MSG << "() - there are still buffers ready for compositor. starting frame dropper";
                QMetaObject::invokeMethod(&m_frameDropperTimer, "start", Qt::QueuedConnection);
            }
        } else {
            // Just get a pointer to the buffer. This tells mir we consumed it.
            renderables[0]->buffer();
        }

        Q_EMIT frameDropped();

    } else {
        WARNING_MSG << "() - failed. Giving up.";
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

        if (texture->textureSize() != size()) {
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

void MirSurface::setFocused(bool value)
{
    if (m_focused == value)
        return;

    INFO_MSG << "(" << value << ")";

    m_focused = value;
    Q_EMIT focusedChanged(value);
}

void MirSurface::setViewActiveFocus(qintptr viewId, bool value)
{
    if (value && !m_activelyFocusedViews.contains(viewId)) {
        m_activelyFocusedViews.insert(viewId);
        updateActiveFocus();
    } else if (!value && (m_activelyFocusedViews.contains(viewId) || m_neverSetSurfaceFocus)) {
        m_activelyFocusedViews.remove(viewId);
        updateActiveFocus();
    }
}

bool MirSurface::activeFocus() const
{
    return !m_activelyFocusedViews.empty();
}

void MirSurface::updateActiveFocus()
{
    if (!m_session) {
        return;
    }

    // Temporary hotfix for http://pad.lv/1483752
    if (m_session->childSessions()->rowCount() > 0) {
        // has child trusted session, ignore any focus change attempts
        INFO_MSG << "() has child trusted session, ignore any focus change attempts";
        return;
    }

    // TODO Figure out what to do here
    /*
    if (m_activelyFocusedViews.isEmpty()) {
        DEBUG_MSG << "() unfocused";
        m_controller->setActiveFocus(m_window, false);
    } else {
        DEBUG_MSG << "() focused";
        m_controller->setActiveFocus(m_window, true);
    }
    */

    m_neverSetSurfaceFocus = false;
}

void MirSurface::updateVisible()
{
    const bool visible = !(m_state == Mir::HiddenState || m_state == Mir::MinimizedState) && m_surface->visible();

    if (m_visible != visible) {
        m_visible = visible;
        Q_EMIT visibleChanged(visible);
    }
}

void MirSurface::close()
{
    if (m_closingState != NotClosing) {
        return;
    }

    INFO_MSG << "()";

    m_closingState = Closing;
    Q_EMIT closeRequested();
    m_closeTimer->start();

    if (m_window) {
        m_controller->requestClose(m_window);
    }
}

void MirSurface::resize(int width, int height)
{
    if (!clientIsRunning()) {
        m_pendingResize = QSize(width, height);
        return;
    }

    bool mirSizeIsDifferent = width != m_size.width() || height != m_size.height();

    if (mirSizeIsDifferent) {
        m_controller->resize(m_window, QSize(width, height));
        DEBUG_MSG << " old (" << m_size.width() << "," << m_size.height() << ")"
                  << ", new (" << width << "," << height << ")";
    }
}

QPoint MirSurface::position() const
{
    return m_position;
}

void MirSurface::setPosition(const QPoint newDisplayPosition)
{
    QPoint newPosition = convertDisplayToLocalCoords(newDisplayPosition);
    if (m_position != newPosition) {
        m_position = newPosition;
        Q_EMIT positionChanged(newPosition);
    }
}

QSize MirSurface::size() const
{
    return m_size;
}

Mir::State MirSurface::state() const
{
    return m_state;
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

QString MirSurface::persistentId() const
{
    return m_extraInfo->persistentId;
}

void MirSurface::requestState(Mir::State state)
{
    INFO_MSG << "(" << unityapiMirStateToStr(state) << ")";
    m_controller->requestState(m_window, state);
}

void MirSurface::setLive(bool value)
{
    if (value != m_live) {
        INFO_MSG << "(" << value << ")";
        m_live = value;
        Q_EMIT liveChanged(value);
        if (m_views.isEmpty() && !m_live) {
            deleteLater();
        }
    }
}

bool MirSurface::live() const
{
    return m_live;
}

bool MirSurface::visible() const
{
    return m_visible;
}
#include <mir_toolkit/event.h>
void MirSurface::mousePressEvent(QMouseEvent *event)
{
    auto ev = EventBuilder::instance()->reconstructMirEvent(event);
    auto ev1 = reinterpret_cast<MirPointerEvent const*>(ev.get());
    m_controller->deliverPointerEvent(m_window, ev1);
    event->accept();
}

void MirSurface::mouseMoveEvent(QMouseEvent *event)
{
    auto ev = EventBuilder::instance()->reconstructMirEvent(event);
    auto ev1 = reinterpret_cast<MirPointerEvent const*>(ev.get());
    m_controller->deliverPointerEvent(m_window, ev1);
    event->accept();
}

void MirSurface::mouseReleaseEvent(QMouseEvent *event)
{
    auto ev = EventBuilder::instance()->reconstructMirEvent(event);
    auto ev1 = reinterpret_cast<MirPointerEvent const*>(ev.get());
    m_controller->deliverPointerEvent(m_window, ev1);
    event->accept();
}

void MirSurface::hoverEnterEvent(QHoverEvent *event)
{
    auto ev = EventBuilder::instance()->reconstructMirEvent(event);
    auto ev1 = reinterpret_cast<MirPointerEvent const*>(ev.get());
    m_controller->deliverPointerEvent(m_window, ev1);
    event->accept();
}

void MirSurface::hoverLeaveEvent(QHoverEvent *event)
{
    auto ev = EventBuilder::instance()->reconstructMirEvent(event);
    auto ev1 = reinterpret_cast<MirPointerEvent const*>(ev.get());
    m_controller->deliverPointerEvent(m_window, ev1);
    event->accept();
}

void MirSurface::hoverMoveEvent(QHoverEvent *event)
{
    auto ev = EventBuilder::instance()->reconstructMirEvent(event);
    auto ev1 = reinterpret_cast<MirPointerEvent const*>(ev.get());
    m_controller->deliverPointerEvent(m_window, ev1);
    event->accept();
}

void MirSurface::wheelEvent(QWheelEvent *event)
{
    auto ev = EventBuilder::instance()->makeMirEvent(event);
    auto ev1 = reinterpret_cast<MirPointerEvent const*>(ev.get());
    m_controller->deliverPointerEvent(m_window, ev1);
    event->accept();
}

void MirSurface::keyPressEvent(QKeyEvent *qtEvent)
{
    auto ev = EventBuilder::instance()->makeMirEvent(qtEvent);
    auto ev1 = reinterpret_cast<MirKeyboardEvent const*>(ev.get());
    m_controller->deliverKeyboardEvent(m_window, ev1);
    qtEvent->accept();
}

void MirSurface::keyReleaseEvent(QKeyEvent *qtEvent)
{
    auto ev = EventBuilder::instance()->makeMirEvent(qtEvent);
    auto ev1 = reinterpret_cast<MirKeyboardEvent const*>(ev.get());
    m_controller->deliverKeyboardEvent(m_window, ev1);
    qtEvent->accept();
}

void MirSurface::touchEvent(Qt::KeyboardModifiers mods,
                            const QList<QTouchEvent::TouchPoint> &touchPoints,
                            Qt::TouchPointStates touchPointStates,
                            ulong timestamp)
{
    auto ev = EventBuilder::instance()->makeMirEvent(mods, touchPoints, touchPointStates, timestamp);
    auto ev1 = reinterpret_cast<MirTouchEvent const*>(ev.get());
    m_controller->deliverTouchEvent(m_window, ev1);
}

bool MirSurface::clientIsRunning() const
{
    return (m_session &&
            (m_session->state() == SessionInterface::State::Running
             || m_session->state() == SessionInterface::State::Starting
             || m_session->state() == SessionInterface::State::Suspending))
        || !m_session;
}

bool MirSurface::isBeingDisplayed() const
{
    return !m_views.isEmpty();
}

void MirSurface::registerView(qintptr viewId)
{
    m_views.insert(viewId, MirSurface::View{false});
    INFO_MSG << "(" << viewId << ")" << " after=" << m_views.count();
    if (m_views.count() == 1) {
        Q_EMIT isBeingDisplayedChanged();
    }
}

void MirSurface::unregisterView(qintptr viewId)
{
    m_views.remove(viewId);
    INFO_MSG << "(" << viewId << ")" << " after=" << m_views.count() << " live=" << m_live;
    if (m_views.count() == 0) {
        Q_EMIT isBeingDisplayedChanged();
        if (m_session.isNull() || !m_live) {
            deleteLater();
        }
    }
    updateExposure();
    setViewActiveFocus(viewId, false);
}

void MirSurface::setViewExposure(qintptr viewId, bool exposed)
{
    if (!m_views.contains(viewId)) return;

    m_views[viewId].exposed = exposed;
    updateExposure();
}

void MirSurface::updateExposure()
{
    // Only update exposure after client has swapped a frame (aka surface is "ready"). MirAL only considers
    // a surface visible after it has drawn something
    if (!m_ready) {
        return;
    }

    bool newExposed = false;
    QHashIterator<qintptr, View> i(m_views);
    while (i.hasNext()) {
        i.next();
        newExposed |= i.value().exposed;
    }

    const bool oldExposed = (m_surface->query(mir_window_attrib_visibility) == mir_window_visibility_exposed);

    if (newExposed != oldExposed) {
        INFO_MSG << "(" << newExposed << ")";

        m_surface->configure(mir_window_attrib_visibility,
                             newExposed ? mir_window_visibility_exposed : mir_window_visibility_occluded);
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
    } else if (m_session) {
        appId = m_session->name();
    } else {
        appId.append("-");
    }
    return appId;
}

void MirSurface::setKeymap(const QString &layoutPlusVariant)
{
    if (m_keymap == layoutPlusVariant) {
        return;
    }

    INFO_MSG << "(" << layoutPlusVariant << ")";

    m_keymap = layoutPlusVariant;
    Q_EMIT keymapChanged(m_keymap);

    applyKeymap();
}

QString MirSurface::keymap() const
{
    return m_keymap;
}

void MirSurface::applyKeymap()
{
    QStringList stringList = m_keymap.split('+', QString::SkipEmptyParts);

    QString layout = stringList[0];
    QString variant;

    if (stringList.count() > 1) {
        variant = stringList[1];
    }

    if (layout.isEmpty()) {
        WARNING_MSG << "Setting keymap with empty layout is not supported";
        return;
    }

    m_surface->set_keymap(MirInputDeviceId(), "", layout.toStdString(), variant.toStdString(), "");
}

QCursor MirSurface::cursor() const
{
    return m_cursor;
}

Mir::ShellChrome MirSurface::shellChrome() const
{
    return m_shellChrome;
}

void MirSurface::setShellChrome(Mir::ShellChrome shellChrome)
{
    if (m_shellChrome != shellChrome) {
        m_shellChrome = shellChrome;

        Q_EMIT shellChromeChanged(shellChrome);
    }
}

bool MirSurface::inputAreaContains(const QPoint &point) const
{
    bool result;


    // Can't use it due to https://bugs.launchpad.net/mir/+bug/1598936
    // FIXME: Use the line below instead of m_inputBounds once this bug gets fixed.
    //result = m_surface->input_area_contains(mir::geometry::Point(point.x(), point.y()));

    if (m_inputBounds.isNull()) {
        result = true;
    } else {
        result = m_inputBounds.contains(point);
    }

    return result;
}

void MirSurface::updateState(Mir::State newState)
{
    if (newState == m_state) {
        return;
    }
    INFO_MSG << "(" << unityapiMirStateToStr(newState) << ")";

    m_state = newState;
    Q_EMIT stateChanged(state());

    // Mir determines visibility from the state, it may have changed
    updateVisible();
}

void MirSurface::setReady()
{
    if (!m_ready) {
        INFO_MSG << "()";
        m_ready = true;
        updateVisible(); // as Mir can change m_surface->visible() to true after first frame swap
        Q_EMIT ready();
        updateExposure();
    }
}

void MirSurface::setCursor(const QCursor &cursor)
{
    INFO_MSG << "(" << qtCursorShapeToStr(cursor.shape()) << ")";

    m_cursor = cursor;
    Q_EMIT cursorChanged(m_cursor);
}

int MirSurface::minimumWidth() const
{
    return m_minWidth;
}

int MirSurface::minimumHeight() const
{
    return m_minHeight;
}

int MirSurface::maximumWidth() const
{
    return m_maxWidth;
}

int MirSurface::maximumHeight() const
{
    return m_maxHeight;
}

int MirSurface::widthIncrement() const
{
    return m_incWidth;
}

int MirSurface::heightIncrement() const
{
    return m_incHeight;
}

bool MirSurface::focused() const
{
    return m_focused;
}

QRect MirSurface::inputBounds() const
{
    return m_inputBounds;
}

bool MirSurface::confinesMousePointer() const
{
    return m_surface->confine_pointer_state() == mir_pointer_confined_to_window;
}

void MirSurface::activate()
{
    INFO_MSG << "()";
    if (m_live) {
        m_controller->activate(m_window);
    }
}

void MirSurface::onCloseTimedOut()
{
    Q_ASSERT(m_closingState == Closing);

    INFO_MSG << "()";

    m_closingState = CloseOverdue;

    if (m_live) {
        m_controller->forceClose(m_window);
    }
}

void MirSurface::setCloseTimer(AbstractTimer *timer)
{
    bool timerWasRunning = false;

    if (m_closeTimer) {
        timerWasRunning = m_closeTimer->isRunning();
        delete m_closeTimer;
    }

    m_closeTimer = timer;
    m_closeTimer->setInterval(3000);
    m_closeTimer->setSingleShot(true);
    connect(m_closeTimer, &AbstractTimer::timeout, this, &MirSurface::onCloseTimedOut);

    if (timerWasRunning) {
        m_closeTimer->start();
    }
}

std::shared_ptr<SurfaceObserver> MirSurface::surfaceObserver() const
{
    return m_surfaceObserver;
}

void MirSurface::setInputBounds(const QRect &rect)
{
    if (m_inputBounds != rect) {
        DEBUG_MSG << "(" << rect << ")";
        m_inputBounds = rect;
        Q_EMIT inputBoundsChanged(m_inputBounds);
    }
}

QPoint MirSurface::requestedPosition() const
{
    return m_requestedPosition;
}

void MirSurface::setRequestedPosition(const QPoint &point)
{
    if (point != m_requestedPosition) {
        DEBUG_MSG << "(" << point << ")";
        m_requestedPosition = point;
        Q_EMIT requestedPositionChanged(m_requestedPosition);

        if (m_live) {
            m_controller->move(m_window, convertLocalToDisplayCoords(m_requestedPosition));
        }
    }
}

void MirSurface::onMinimumWidthChanged(int minWidth)
{
    if (m_minWidth != minWidth)
    {
        m_minWidth = minWidth;
        Q_EMIT minimumWidthChanged(minWidth);
    }
}

void MirSurface::onMinimumHeightChanged(int minHeight)
{
    if (m_minHeight != minHeight)
    {
        m_minHeight = minHeight;
        Q_EMIT minimumHeightChanged(minHeight);
    }
}

void MirSurface::onMaximumWidthChanged(int maxWidth)
{
    if (m_maxWidth != maxWidth)
    {
        m_maxWidth = maxWidth;
        Q_EMIT maximumWidthChanged(maxWidth);
    }
}

void MirSurface::onMaximumHeightChanged(int maxHeight)
{
    if (m_maxHeight != maxHeight)
    {
        m_maxHeight = maxHeight;
        Q_EMIT maximumHeightChanged(maxHeight);
    }
}

void MirSurface::onWidthIncrementChanged(int incWidth)
{
    if (m_incWidth != incWidth)
    {
        m_incWidth = incWidth;
        Q_EMIT widthIncrementChanged(incWidth);
    }
}

void MirSurface::onHeightIncrementChanged(int incHeight)
{
    if (m_incHeight != incHeight)
    {
        m_incHeight = incHeight;
        Q_EMIT heightIncrementChanged(incHeight);
    }
}

void MirSurface::onNameChanged(const QString &name)
{
    if (m_name != name)
    {
        m_name = name;
        Q_EMIT nameChanged(name);
    }
}

MirSurface::SurfaceObserverImpl::SurfaceObserverImpl()
    : m_listener(nullptr)
    , m_framesPosted(false)
{
    m_cursorNameToShape["left_ptr"] = Qt::ArrowCursor;
    m_cursorNameToShape["up_arrow"] = Qt::UpArrowCursor;
    m_cursorNameToShape["cross"] = Qt::CrossCursor;
    m_cursorNameToShape["watch"] = Qt::WaitCursor;
    m_cursorNameToShape["xterm"] = Qt::IBeamCursor;
    m_cursorNameToShape["size_ver"] = Qt::SizeVerCursor;
    m_cursorNameToShape["size_hor"] = Qt::SizeHorCursor;
    m_cursorNameToShape["size_bdiag"] = Qt::SizeBDiagCursor;
    m_cursorNameToShape["size_fdiag"] = Qt::SizeFDiagCursor;
    m_cursorNameToShape["size_all"] = Qt::SizeAllCursor;
    m_cursorNameToShape["blank"] = Qt::BlankCursor;
    m_cursorNameToShape["split_v"] = Qt::SplitVCursor;
    m_cursorNameToShape["split_h"] = Qt::SplitHCursor;
    m_cursorNameToShape["hand"] = Qt::PointingHandCursor;
    m_cursorNameToShape["forbidden"] = Qt::ForbiddenCursor;
    m_cursorNameToShape["whats_this"] = Qt::WhatsThisCursor;
    m_cursorNameToShape["left_ptr_watch"] = Qt::BusyCursor;
    m_cursorNameToShape["openhand"] = Qt::OpenHandCursor;
    m_cursorNameToShape["closedhand"] = Qt::ClosedHandCursor;
    m_cursorNameToShape["dnd-copy"] = Qt::DragCopyCursor;
    m_cursorNameToShape["dnd-move"] = Qt::DragMoveCursor;
    m_cursorNameToShape["dnd-link"] = Qt::DragLinkCursor;

    // Used by Mir client API (mir_*_cursor_name strings)
    m_cursorNameToShape["default"] = Qt::ArrowCursor;
    m_cursorNameToShape["disabled"] = Qt::BlankCursor;
    m_cursorNameToShape["arrow"] = Qt::ArrowCursor;
    m_cursorNameToShape["busy"] = Qt::WaitCursor;
    m_cursorNameToShape["caret"] = Qt::IBeamCursor;
    m_cursorNameToShape["pointing-hand"] = Qt::PointingHandCursor;
    m_cursorNameToShape["open-hand"] = Qt::OpenHandCursor;
    m_cursorNameToShape["closed-hand"] = Qt::ClosedHandCursor;
    m_cursorNameToShape["horizontal-resize"] = Qt::SizeHorCursor;
    m_cursorNameToShape["vertical-resize"] = Qt::SizeVerCursor;
    m_cursorNameToShape["diagonal-resize-bottom-to-top"] = Qt::SizeBDiagCursor;
    m_cursorNameToShape["diagonal-resize-top_to_bottom"] = Qt::SizeFDiagCursor; // current string with typo
    m_cursorNameToShape["diagonal-resize-top-to-bottom"] = Qt::SizeFDiagCursor; // how it will be when they fix it (if ever)
    m_cursorNameToShape["omnidirectional-resize"] = Qt::SizeAllCursor;
    m_cursorNameToShape["vsplit-resize"] = Qt::SplitVCursor;
    m_cursorNameToShape["hsplit-resize"] = Qt::SplitHCursor;
    m_cursorNameToShape["crosshair"] = Qt::CrossCursor;

    qRegisterMetaType<MirShellChrome>("MirShellChrome");
}

MirSurface::SurfaceObserverImpl::~SurfaceObserverImpl()
{
}

void MirSurface::SurfaceObserverImpl::setListener(QObject *listener)
{
    m_listener = listener;
    if (m_framesPosted) {
        Q_EMIT framesPosted();
    }
}

void MirSurface::SurfaceObserverImpl::frame_posted(int /*frames_available*/, mir::geometry::Size const& /*size*/)
{
    m_framesPosted = true;
    if (m_listener) {
        Q_EMIT framesPosted();
    }
}

void MirSurface::SurfaceObserverImpl::renamed(char const * name)
{
    Q_EMIT nameChanged(QString::fromUtf8(name));
}

void MirSurface::SurfaceObserverImpl::cursor_image_removed()
{
    Q_EMIT cursorChanged(QCursor());
}

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 25, 0)
void MirSurface::SurfaceObserverImpl::placed_relative(mir::geometry::Rectangle const& /*placement*/)
{
}
#endif

void MirSurface::SurfaceObserverImpl::attrib_changed(MirWindowAttrib attribute, int value)
{
    if (m_listener) {
        Q_EMIT attributeChanged(attribute, value);
    }
}

void MirSurface::SurfaceObserverImpl::resized_to(mir::geometry::Size const&size)
{
    Q_EMIT resized(QSize(size.width.as_int(), size.height.as_int()));
}

void MirSurface::SurfaceObserverImpl::cursor_image_set_to(const mir::graphics::CursorImage &cursorImage)
{
    QCursor qcursor = createQCursorFromMirCursorImage(cursorImage);
    Q_EMIT cursorChanged(qcursor);
}

QCursor MirSurface::SurfaceObserverImpl::createQCursorFromMirCursorImage(const mir::graphics::CursorImage &cursorImage) {
    if (cursorImage.as_argb_8888() == nullptr) {
        // Must be a named cursor
        auto namedCursor = dynamic_cast<const qtmir::NamedCursor*>(&cursorImage);
        Q_ASSERT(namedCursor != nullptr);
        if (namedCursor) {
            // NB: If we need a named cursor not covered by Qt::CursorShape, we won't be able to
            //     used Qt's cursor API anymore for transmitting MirSurface's cursor image.

            Qt::CursorShape cursorShape = Qt::ArrowCursor;
            {
                auto iterator = m_cursorNameToShape.constFind(namedCursor->name());
                if (iterator == m_cursorNameToShape.constEnd()) {
                    qCWarning(QTMIR_SURFACES).nospace() << "SurfaceObserver: unrecognized cursor name "
                                                        << namedCursor->name();
                } else {
                    cursorShape = iterator.value();
                }
            }
            return QCursor(cursorShape);
        } else {
            // shouldn't happen
            return QCursor();
        }
    } else {
        QImage image((const uchar*)cursorImage.as_argb_8888(),
                     cursorImage.size().width.as_int(), cursorImage.size().height.as_int(), QImage::Format_ARGB32);

        return QCursor(QPixmap::fromImage(image), cursorImage.hotspot().dx.as_int(), cursorImage.hotspot().dy.as_int());
    }
}

void MirSurface::requestFocus()
{
    INFO_MSG << "()";
    Q_EMIT focusRequested();
}

QPoint MirSurface::convertDisplayToLocalCoords(const QPoint &displayPos) const
{
    QPoint localPos = displayPos;

    if (m_surface->parent()) {
        auto parentPos = m_surface->parent()->top_left();
        localPos.rx() -= parentPos.x.as_int();
        localPos.ry() -= parentPos.y.as_int();
    }

    return localPos;
}

QPoint MirSurface::convertLocalToDisplayCoords(const QPoint &localPos) const
{
    QPoint displayPos = localPos;

    if (m_surface->parent()) {
        auto parentPos = m_surface->parent()->top_left();
        displayPos.rx() += parentPos.x.as_int();
        displayPos.ry() += parentPos.y.as_int();
    }

    return displayPos;
}

unityapi::MirSurfaceInterface *MirSurface::parentSurface() const
{
    return m_parentSurface;
}

unityapi::MirSurfaceListInterface *MirSurface::childSurfaceList() const
{
    return m_childSurfaceList;
}
