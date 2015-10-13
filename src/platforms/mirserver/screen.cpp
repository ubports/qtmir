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

// local
#include "screen.h"
#include "logging.h"

// Mir
#include "mir/geometry/size.h"
#include "mir/graphics/buffer.h"
#include "mir/graphics/display_buffer.h"
#include "mir/graphics/display.h"

// Qt
#include <QCoreApplication>
#include <qpa/qwindowsysteminterface.h>
#include <QThread>

// Qt sensors
#include <QtSensors/QOrientationReading>
#include <QtSensors/QOrientationSensor>

namespace mg = mir::geometry;

Q_LOGGING_CATEGORY(QTMIR_SENSOR_MESSAGES, "qtmir.sensor")

namespace {
bool isLittleEndian() {
    unsigned int i = 1;
    char *c = (char*)&i;
    return *c == 1;
}

enum QImage::Format qImageFormatFromMirPixelFormat(MirPixelFormat mirPixelFormat) {
    switch (mirPixelFormat) {
    case mir_pixel_format_abgr_8888:
        if (isLittleEndian()) {
            // 0xRR,0xGG,0xBB,0xAA
            return QImage::Format_RGBA8888;
        } else {
            // 0xAA,0xBB,0xGG,0xRR
            qFatal("[mirserver QPA] "
                   "Qt doesn't support mir_pixel_format_abgr_8888 in a big endian architecture");
        }
        break;
    case mir_pixel_format_xbgr_8888:
        if (isLittleEndian()) {
            // 0xRR,0xGG,0xBB,0xXX
            return QImage::Format_RGBX8888;
        } else {
            // 0xXX,0xBB,0xGG,0xRR
            qFatal("[mirserver QPA] "
                   "Qt doesn't support mir_pixel_format_xbgr_8888 in a big endian architecture");
        }
        break;
        break;
    case mir_pixel_format_argb_8888:
        // 0xAARRGGBB
        return QImage::Format_ARGB32;
        break;
    case mir_pixel_format_xrgb_8888:
        // 0xffRRGGBB
        return QImage::Format_RGB32;
        break;
    case mir_pixel_format_bgr_888:
        qFatal("[mirserver QPA] Qt doesn't support mir_pixel_format_bgr_888");
        break;
    default:
        qFatal("[mirserver QPA] Unknown mir pixel format");
        break;
    }
}

} // namespace {


class OrientationReadingEvent : public QEvent {
public:
    OrientationReadingEvent(QEvent::Type type, QOrientationReading::Orientation orientation)
        : QEvent(type)
        , m_orientation(orientation) {
    }

    static const QEvent::Type m_type;
    QOrientationReading::Orientation m_orientation;
};

const QEvent::Type OrientationReadingEvent::m_type =
        static_cast<QEvent::Type>(QEvent::registerEventType());

bool Screen::skipDBusRegistration = false;

Screen::Screen(const mir::graphics::DisplayConfigurationOutput &screen)
    : QObject(nullptr)
    , m_displayBuffer(nullptr)
    , m_displayGroup(nullptr)
    , m_orientationSensor(new QOrientationSensor(this))
    , m_screenWindow(nullptr)
    , m_unityScreen(nullptr)
{
    setMirDisplayConfiguration(screen);

    // Set the default orientation based on the initial screen dimmensions.
    m_nativeOrientation = (m_geometry.width() >= m_geometry.height())
        ? Qt::LandscapeOrientation : Qt::PortraitOrientation;
    qCDebug(QTMIR_SENSOR_MESSAGES) << "Screen - nativeOrientation is:" << m_nativeOrientation;

    // If it's a landscape device (i.e. some tablets), start in landscape, otherwise portrait
    m_currentOrientation = (m_nativeOrientation == Qt::LandscapeOrientation)
            ? Qt::LandscapeOrientation : Qt::PortraitOrientation;
    qCDebug(QTMIR_SENSOR_MESSAGES) << "Screen - initial currentOrientation is:" << m_currentOrientation;

    QObject::connect(m_orientationSensor, &QOrientationSensor::readingChanged,
                     this, &Screen::onOrientationReadingChanged);
    m_orientationSensor->start();

    if (!skipDBusRegistration) {
        // FIXME This is a unity8 specific dbus call and shouldn't be in qtmir
        m_unityScreen = new QDBusInterface("com.canonical.Unity.Screen",
                                         "/com/canonical/Unity/Screen",
                                         "com.canonical.Unity.Screen",
                                         QDBusConnection::systemBus(), this);

        m_unityScreen->connection().connect("com.canonical.Unity.Screen",
                                          "/com/canonical/Unity/Screen",
                                          "com.canonical.Unity.Screen",
                                          "DisplayPowerStateChange",
                                          this,
                                          SLOT(onDisplayPowerStateChanged(int, int)));
    }
}

Screen::~Screen()
{
    //if a ScreenWindow associated with this screen, kill it
    if (m_screenWindow) {
        m_screenWindow->window()->destroy(); // ends up destroying m_ScreenWindow
    }
}

bool Screen::orientationSensorEnabled()
{
    return m_orientationSensor->isActive();
}

void Screen::onDisplayPowerStateChanged(int status, int reason)
{
    Q_UNUSED(reason);
    toggleSensors(status);
}

void Screen::setMirDisplayConfiguration(const mir::graphics::DisplayConfigurationOutput &screen)
{
    // Note: DisplayConfigurationOutput will be destroyed after this function returns

    // Output data - each output has a unique id and corresponding type. Can be multiple cards.
    m_outputId = screen.id;
    m_cardId = screen.card_id;
    m_type = screen.type;

    // Physical screen size
    m_physicalSize.setWidth(screen.physical_size_mm.width.as_float());
    m_physicalSize.setHeight(screen.physical_size_mm.height.as_float());

    // Pixel Format
    m_format = qImageFormatFromMirPixelFormat(screen.current_format);

    // Pixel depth
    m_depth = 8 * MIR_BYTES_PER_PIXEL(screen.current_format);

    // Power mode
    m_powerMode = screen.power_mode;

    QRect oldGeometry = m_geometry;
    // Position of screen in virtual desktop coordinate space
    m_geometry.setTop(screen.top_left.y.as_int());
    m_geometry.setLeft(screen.top_left.x.as_int());

    // Mode = current resolution & refresh rate
    mir::graphics::DisplayConfigurationMode mode = screen.modes.at(screen.current_mode_index);
    m_geometry.setWidth(mode.size.width.as_int());
    m_geometry.setHeight(mode.size.height.as_int());

    // DPI - unnecessary to calculate, default implementation in QPlatformScreen is sufficient

    // Check for Screen geometry change
    if (m_geometry != oldGeometry) {
        QWindowSystemInterface::handleScreenGeometryChange(this->screen(), m_geometry, m_geometry);
        if (m_screenWindow) { // resize corresponding window immediately
            m_screenWindow->setGeometry(m_geometry);
        }
    }

    // Refresh rate
    if (m_refreshRate != mode.vrefresh_hz) {
        m_refreshRate = mode.vrefresh_hz;
        QWindowSystemInterface::handleScreenRefreshRateChange(this->screen(), mode.vrefresh_hz);
    }
}

void Screen::toggleSensors(const bool enable) const
{
    qCDebug(QTMIR_SENSOR_MESSAGES) << "Screen::toggleSensors - enable=" << enable;
    if (enable) {
        m_orientationSensor->start();
    } else {
        m_orientationSensor->stop();
    }
}

void Screen::customEvent(QEvent* event)
{
    OrientationReadingEvent* oReadingEvent = static_cast<OrientationReadingEvent*>(event);
    switch (oReadingEvent->m_orientation) {
        case QOrientationReading::LeftUp: {
            m_currentOrientation = (m_nativeOrientation == Qt::LandscapeOrientation) ?
                        Qt::InvertedPortraitOrientation : Qt::LandscapeOrientation;
            break;
        }
        case QOrientationReading::TopUp: {
            m_currentOrientation = (m_nativeOrientation == Qt::LandscapeOrientation) ?
                        Qt::LandscapeOrientation : Qt::PortraitOrientation;
            break;
        }
        case QOrientationReading::RightUp: {
            m_currentOrientation = (m_nativeOrientation == Qt::LandscapeOrientation) ?
                        Qt::PortraitOrientation : Qt::InvertedLandscapeOrientation;
            break;
        }
        case QOrientationReading::TopDown: {
            m_currentOrientation = (m_nativeOrientation == Qt::LandscapeOrientation) ?
                        Qt::InvertedLandscapeOrientation : Qt::InvertedPortraitOrientation;
            break;
        }
        default: {
            qWarning("Unknown orientation.");
            event->accept();
            return;
        }
    }

    // Raise the event signal so that client apps know the orientation changed
    QWindowSystemInterface::handleScreenOrientationChange(screen(), m_currentOrientation);
    event->accept();
    qCDebug(QTMIR_SENSOR_MESSAGES) << "Screen::customEvent - new orientation" << m_currentOrientation << "handled";
}

void Screen::onOrientationReadingChanged()
{
    qCDebug(QTMIR_SENSOR_MESSAGES) << "Screen::onOrientationReadingChanged";

    // Make sure to switch to the main Qt thread context
    QCoreApplication::postEvent(this, new OrientationReadingEvent(
                                              OrientationReadingEvent::m_type,
                                              m_orientationSensor->reading()->orientation()));
}

QPlatformCursor *Screen::cursor() const
{
    const QPlatformCursor *platformCursor = &m_cursor;
    return const_cast<QPlatformCursor *>(platformCursor);
}

ScreenWindow *Screen::window() const
{
    return m_screenWindow;
}

void Screen::setWindow(ScreenWindow *window)
{
    if (window && m_screenWindow) {
        qCDebug(QTMIR_SENSOR_MESSAGES) << "Screen::setWindow - overwriting existing ScreenWindow";
    }
    m_screenWindow = window;
}

void Screen::setMirDisplayBuffer(mir::graphics::DisplayBuffer *buffer, mir::graphics::DisplaySyncGroup *group)
{
    qCDebug(QTMIR_SCREENS) << "Screen::setMirDisplayBuffer" << buffer << group;
    // This operation should only be performed while rendering is stopped
    m_displayBuffer = buffer;
    m_displayGroup = group;
}

void Screen::swapBuffers()
{
    m_displayBuffer->gl_swap_buffers();

    /* FIXME this exposes a QtMir architecture problem, as Screen is supposed to wrap a mg::DisplayBuffer.
     * We use Qt's multithreaded renderer, where each Screen is rendered to relatively independently, and
     * post() called also individually.
     *
     * But if this is a native server on Android, in the multimonitor case a DisplaySyncGroup can contain
     * 2+ DisplayBuffers, one post() call will submit all mg::DisplayBuffers in the group for flipping.
     * This will cause just one Screen to be updated, blocking the swap call for the other Screens, which
     * will slow rendering dramatically.
     *
     * Integrating the Qt Scenegraph renderer as a Mir renderer should solve this issue.
     */
    m_displayGroup->post();
}

void Screen::makeCurrent()
{
    m_displayBuffer->make_current();
}

void Screen::doneCurrent()
{
    m_displayBuffer->release_current();
}
