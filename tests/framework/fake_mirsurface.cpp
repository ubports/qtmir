#include "fake_mirsurface.h"

qtmir::FakeMirSurface::TouchEvent::TouchEvent(Qt::KeyboardModifiers mods,
                                              const QList<QTouchEvent::TouchPoint> &points,
                                              Qt::TouchPointStates states,
                                              ulong timestamp)
    : keyboardModifiers(mods)
    , touchPoints(points)
    , states(states)
    , timestamp(timestamp)
{
}

qtmir::FakeMirSurface::TouchEvent::~TouchEvent()
{
}

qtmir::FakeMirSurface::FakeMirSurface(QObject *parent)
    : MirSurfaceInterface(parent)
    , m_isFirstFrameDrawn(false)
    , m_session(nullptr)
    , m_isFrameDropperRunning(true)
    , m_live(true)
    , m_state(Mir::RestoredState)
    , m_orientationAngle(Mir::Angle0)
    , m_viewCount(0)
    , m_focused(false)
{
}

qtmir::FakeMirSurface::~FakeMirSurface()
{
}

Mir::Type qtmir::FakeMirSurface::type() const { return Mir::NormalType; }

QString qtmir::FakeMirSurface::name() const { return QString("Fake MirSurface"); }

QSize qtmir::FakeMirSurface::size() const { return m_size; }

void qtmir::FakeMirSurface::resize(int width, int height) {
    if (m_size.width() != width || m_size.height() != height) {
        m_size.setWidth(width);
        m_size.setHeight(height);
        Q_EMIT sizeChanged(m_size);
    }
}

void qtmir::FakeMirSurface::resize(const QSize &size) { resize(size.width(), size.height()); }

Mir::State qtmir::FakeMirSurface::state() const { return m_state; }

void qtmir::FakeMirSurface::setState(Mir::State qmlState) {
    if (qmlState != m_state) {
        m_state = qmlState;
        Q_EMIT stateChanged(m_state);
    }
}

bool qtmir::FakeMirSurface::live() const { return m_live; }

Mir::OrientationAngle qtmir::FakeMirSurface::orientationAngle() const { return m_orientationAngle; }

void qtmir::FakeMirSurface::setOrientationAngle(Mir::OrientationAngle angle) {
    if (m_orientationAngle != angle) {
        m_orientationAngle = angle;
        Q_EMIT orientationAngleChanged(m_orientationAngle);
    }
}

bool qtmir::FakeMirSurface::isFirstFrameDrawn() const {
    return m_isFirstFrameDrawn;
}

void qtmir::FakeMirSurface::stopFrameDropper() {
    m_isFrameDropperRunning = false;
}

void qtmir::FakeMirSurface::startFrameDropper() {
    m_isFrameDropperRunning = true;
}

void qtmir::FakeMirSurface::setLive(bool value) {
    if (m_live != value) {
        m_live = value;
        Q_EMIT liveChanged(m_live);
    }
}

bool qtmir::FakeMirSurface::isBeingDisplayed() const { return m_viewCount > 0; }

void qtmir::FakeMirSurface::incrementViewCount() {
    ++m_viewCount;
    if (m_viewCount == 1) {
        Q_EMIT isBeingDisplayedChanged();
    }
}

void qtmir::FakeMirSurface::decrementViewCount() {
    --m_viewCount;
    if (m_viewCount == 0) {
        Q_EMIT isBeingDisplayedChanged();
    }
}

QSharedPointer<QSGTexture> qtmir::FakeMirSurface::texture() { return QSharedPointer<QSGTexture>(); }

void qtmir::FakeMirSurface::updateTexture() {}

unsigned int qtmir::FakeMirSurface::currentFrameNumber() const { return 0; }

bool qtmir::FakeMirSurface::numBuffersReadyForCompositor() { return 0; }

void qtmir::FakeMirSurface::setFocus(bool focus) { m_focused = focus; }

void qtmir::FakeMirSurface::mousePressEvent(QMouseEvent *) {}

void qtmir::FakeMirSurface::mouseMoveEvent(QMouseEvent *) {}

void qtmir::FakeMirSurface::mouseReleaseEvent(QMouseEvent *) {}

void qtmir::FakeMirSurface::hoverEnterEvent(QHoverEvent *) {}

void qtmir::FakeMirSurface::hoverLeaveEvent(QHoverEvent *) {}

void qtmir::FakeMirSurface::hoverMoveEvent(QHoverEvent *) {}

void qtmir::FakeMirSurface::keyPressEvent(QKeyEvent *) {}

void qtmir::FakeMirSurface::keyReleaseEvent(QKeyEvent *) {}

void qtmir::FakeMirSurface::touchEvent(Qt::KeyboardModifiers mods, const QList<QTouchEvent::TouchPoint> &points, Qt::TouchPointStates states, ulong timestamp) {
    m_touchesReceived.append(TouchEvent(mods, points, states, timestamp));
}

QString qtmir::FakeMirSurface::appId() const { return "foo-app"; }

void qtmir::FakeMirSurface::onCompositorSwappedBuffers() {}

void qtmir::FakeMirSurface::drawFirstFrame() {
    if (!m_isFirstFrameDrawn) {
        m_isFirstFrameDrawn = true;
        Q_EMIT firstFrameDrawn();
    }
}

bool qtmir::FakeMirSurface::isFrameDropperRunning() const {
    return m_isFrameDropperRunning;
}

QList<qtmir::FakeMirSurface::TouchEvent> &qtmir::FakeMirSurface::touchesReceived() { return m_touchesReceived; }
