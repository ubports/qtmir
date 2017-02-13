/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
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
#include "platformscreen.h"
#include "logging.h"
#include "nativeinterface.h"

// Mir
#include "mir/geometry/size.h"
#include "mir/graphics/buffer.h"
#include "mir/graphics/display_buffer.h"
#include "mir/graphics/display.h"
#include <mir/graphics/display_configuration.h>
#include <mir/renderer/gl/render_target.h>

// Qt
#include <QGuiApplication>
#include <qpa/qwindowsysteminterface.h>
#include <QtMath>

// Qt sensors
#include <QtSensors/QOrientationReading>
#include <QtSensors/QOrientationSensor>

namespace mg = mir::geometry;

#define DEBUG_MSG_SCREENS qCDebug(QTMIR_SCREENS).nospace() << "PlatformScreen[" << (void*)this <<"]::" << __func__
#define DEBUG_MSG_SENSORS qCDebug(QTMIR_SENSOR_MESSAGES).nospace() << "PlatformScreen[" << (void*)this <<"]::" << __func__
#define WARNING_MSG_SENSORS qCWarning(QTMIR_SENSOR_MESSAGES).nospace() << "PlatformScreen[" << (void*)this <<"]::" << __func__

namespace {
bool isLittleEndian() {
    unsigned int i = 1;
    char *c = (char*)&i;
    return *c == 1;
}
static mir::renderer::gl::RenderTarget *as_render_target(
    mir::graphics::DisplayBuffer *displayBuffer)
{
    auto const render_target =
        dynamic_cast<mir::renderer::gl::RenderTarget*>(
            displayBuffer->native_display_buffer());
    if (!render_target)
        throw std::logic_error("DisplayBuffer does not support GL rendering");

    return render_target;
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
    return QImage::Format_Invalid;
}

QString displayTypeToString(qtmir::OutputTypes type)
{
    typedef qtmir::OutputTypes Type;
    switch (type) {
    case Type::VGA:           return QStringLiteral("VGP");
    case Type::DVII:          return QStringLiteral("DVI-I");
    case Type::DVID:          return QStringLiteral("DVI-D");
    case Type::DVIA:          return QStringLiteral("DVI-A");
    case Type::Composite:     return QStringLiteral("Composite");
    case Type::SVideo:        return QStringLiteral("S-Video");
    case Type::LVDS:          return QStringLiteral("LVDS");
    case Type::Component:     return QStringLiteral("Component");
    case Type::NinePinDIN:    return QStringLiteral("9 Pin DIN");
    case Type::DisplayPort:   return QStringLiteral("DisplayPort");
    case Type::HDMIA:         return QStringLiteral("HDMI-A");
    case Type::HDMIB:         return QStringLiteral("HDMI-B");
    case Type::TV:            return QStringLiteral("TV");
    case Type::EDP:           return QStringLiteral("EDP");
    case Type::Unknown:
    default:
        return QStringLiteral("Unknown");
    } //switch
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

bool PlatformScreen::skipDBusRegistration = false;

PlatformScreen::PlatformScreen(const mir::graphics::DisplayConfigurationOutput &screen)
    : QObject(nullptr)
    , m_used(false)
    , m_refreshRate(-1.0)
    , m_scale(1.0)
    , m_formFactor(qtmir::FormFactorUnknown)
    , m_isActive(false)
    , m_renderTarget(nullptr)
    , m_displayGroup(nullptr)
    , m_orientationSensor(new QOrientationSensor(this))
    , m_unityScreen(nullptr)
{
    setMirDisplayConfiguration(screen, false);
    DEBUG_MSG_SCREENS << "(output=" << m_outputId.as_value()
                      << ", used=" << (m_used ? "true" : "false")
                      << ", geometry=" << geometry() << ")";

    // Set the default orientation based on the initial screen dimmensions.
    m_nativeOrientation = (m_geometry.width() >= m_geometry.height())
        ? Qt::LandscapeOrientation : Qt::PortraitOrientation;
    qCDebug(QTMIR_SENSOR_MESSAGES) << "Screen - nativeOrientation is:" << m_nativeOrientation;

    // If it's a landscape device (i.e. some tablets), start in landscape, otherwise portrait
    m_currentOrientation = (m_nativeOrientation == Qt::LandscapeOrientation)
            ? Qt::LandscapeOrientation : Qt::PortraitOrientation;
    qCDebug(QTMIR_SENSOR_MESSAGES) << "Screen - initial currentOrientation is:" << m_currentOrientation;

    if (internalDisplay()) { // only enable orientation sensor for device-internal display
        QObject::connect(m_orientationSensor, &QOrientationSensor::readingChanged,
                         this, &PlatformScreen::onOrientationReadingChanged);
        m_orientationSensor->start();
    }

    if (!skipDBusRegistration) {
        // FIXME This is a unity8 specific dbus call and shouldn't be in qtmir
        m_unityScreen = new QDBusInterface(QStringLiteral("com.canonical.Unity.Screen"),
                                         QStringLiteral("/com/canonical/Unity/Screen"),
                                         QStringLiteral("com.canonical.Unity.Screen"),
                                         QDBusConnection::systemBus(), this);

        m_unityScreen->connection().connect(QStringLiteral("com.canonical.Unity.Screen"),
                                          QStringLiteral("/com/canonical/Unity/Screen"),
                                          QStringLiteral("com.canonical.Unity.Screen"),
                                          QStringLiteral("DisplayPowerStateChange"),
                                          this,
                                          SLOT(onDisplayPowerStateChanged(int, int)));
    }
}

PlatformScreen::~PlatformScreen()
{
    //if a ScreenWindow associated with this screen, kill it
    Q_FOREACH (ScreenPlatformWindow* window, m_screenWindows) {
        window->window()->destroy(); // ends up destroying window
    }
}

bool PlatformScreen::orientationSensorEnabled()
{
    return m_orientationSensor->isActive();
}

void PlatformScreen::setActive(bool active)
{
    if (m_isActive == active)
        return;

    if (active) {
        Q_FOREACH(auto screen, QGuiApplication::screens()) {
            const auto platformScreen = static_cast<PlatformScreen *>(screen->handle());
            if (platformScreen->isActive() && platformScreen != this) {
                platformScreen->setActive(false);
            }
        }
    }
    m_isActive = active;
    Q_EMIT activeChanged(active);
}

void PlatformScreen::onDisplayPowerStateChanged(int status, int reason)
{
    Q_UNUSED(reason);
    if (internalDisplay()) {
        toggleSensors(status);
    }
}

void PlatformScreen::setMirDisplayConfiguration(const mir::graphics::DisplayConfigurationOutput &screen,
                                        bool notify)
{
    // Note: DisplayConfigurationOutput will be destroyed after this function returns

    if (m_used != screen.used) {
        m_used = screen.used;
        Q_EMIT usedChanged();
    }

    // Output data - each output has a unique id and corresponding type. Can be multiple cards.
    m_outputId = screen.id;
    auto type = static_cast<qtmir::OutputTypes>(screen.type); //FIXME: need compile time check these are equivalent
    if (m_type != type) {
        m_type = type;
        Q_EMIT outputTypeChanged();
        Q_EMIT nameChanged();
    }

    // Physical screen size
    QSize physicalSize{screen.physical_size_mm.width.as_int(), screen.physical_size_mm.height.as_int()};
    if (physicalSize != m_physicalSize) {
        m_physicalSize = physicalSize;
        Q_EMIT physicalSizeChanged();
    }

    // Screen capabilities
    uint32_t oldCurrentModeIndex = m_currentModeIndex;
    m_currentModeIndex = screen.current_mode_index;

    // available modes
    QList<PlatformScreen::Mode> availableModes;
    Q_FOREACH(auto mode, screen.modes) {
        availableModes.append(PlatformScreen::Mode{mode.vrefresh_hz,  QSize{mode.size.width.as_int(), mode.size.height.as_int()}});
    }
    if (m_availableModes != availableModes) {
        m_availableModes = availableModes;
        Q_EMIT availableModesChanged();
    }

    if (m_currentModeIndex != oldCurrentModeIndex) {
        Q_EMIT currentModeIndexChanged();
    }

    // Current Pixel Format & depth
    m_format = qImageFormatFromMirPixelFormat(screen.current_format);
    m_depth = 8 * MIR_BYTES_PER_PIXEL(screen.current_format);

    // Power mode
    m_powerMode = screen.power_mode;

    QRect oldGeometry = m_geometry;
    // Position of screen in virtual desktop coordinate space
    m_geometry.setTop(screen.top_left.y.as_int());
    m_geometry.setLeft(screen.top_left.x.as_int());

    // Mode = current resolution & refresh rate
    mir::graphics::DisplayConfigurationMode mode = screen.modes.at(m_currentModeIndex);
    m_geometry.setWidth(mode.size.width.as_int());
    m_geometry.setHeight(mode.size.height.as_int());

    // DPI - unnecessary to calculate, default implementation in QPlatformScreen is sufficient

    // Check for Screen geometry change
    if (m_geometry != oldGeometry) {
        if (notify) {
            QWindowSystemInterface::handleScreenGeometryChange(this->screen(), m_geometry, m_geometry);
        }

        Q_FOREACH (ScreenPlatformWindow* window, m_screenWindows) {
            window->setGeometry(m_geometry);
        }
        if (oldGeometry.topLeft() != m_geometry.topLeft()) {
            Q_EMIT positionChanged();
        }
    }

    // Refresh rate
    if (m_refreshRate != mode.vrefresh_hz) {
        m_refreshRate = mode.vrefresh_hz;
        if (notify) {
            QWindowSystemInterface::handleScreenRefreshRateChange(this->screen(), mode.vrefresh_hz);
        }
    }

    // DPI - unnecessary to calculate, default implementation in QPlatformScreen is sufficient

    // Scale, DPR & Form Factor
    // Update the scale & form factor native-interface properties for the windows affected
    // as there is no convenient way to emit signals for those custom properties on a QScreen
    m_devicePixelRatio = 1.0; //qCeil(m_scale); // FIXME: I need to announce this changing, probably by delete/recreate Screen

    auto formFactor = static_cast<qtmir::FormFactor>(screen.form_factor);
    if (m_formFactor != formFactor) {
        m_formFactor = formFactor;
        Q_EMIT formFactorChanged();
    }

    if (!qFuzzyCompare(screen.scale, m_scale)) {
        m_scale = screen.scale;
        Q_EMIT scaleChanged();
    }
}

void PlatformScreen::toggleSensors(const bool enable) const
{
    DEBUG_MSG_SENSORS << "(enable=" << enable << ")";
    if (enable) {
        m_orientationSensor->start();
    } else {
        m_orientationSensor->stop();
    }
}

void PlatformScreen::customEvent(QEvent* event)
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
            WARNING_MSG_SENSORS << "() - unknown orientation.";
            event->accept();
            return;
        }
    }

    // Raise the event signal so that client apps know the orientation changed
    QWindowSystemInterface::handleScreenOrientationChange(screen(), m_currentOrientation);
    event->accept();
    DEBUG_MSG_SENSORS << "() - new orientation=" << m_currentOrientation;
}

void PlatformScreen::onOrientationReadingChanged()
{
    DEBUG_MSG_SENSORS << "()";

    // Make sure to switch to the main Qt thread context
    QCoreApplication::postEvent(this, new OrientationReadingEvent(
                                              OrientationReadingEvent::m_type,
                                              m_orientationSensor->reading()->orientation()));
}

void PlatformScreen::activate()
{
    setActive(true);
}

QPlatformCursor *PlatformScreen::cursor() const
{
    if (!m_cursor) {
        const_cast<PlatformScreen*>(this)->m_cursor.reset(new qtmir::Cursor);
    }
    return m_cursor.data();
}

QString PlatformScreen::name() const
{
    return displayTypeToString(m_type);
}

QWindow *PlatformScreen::topLevelAt(const QPoint &point) const
{
    QVector<ScreenPlatformWindow*>::const_iterator screen = m_screenWindows.constBegin();
    QVector<ScreenPlatformWindow*>::const_iterator end = m_screenWindows.constEnd();

    while (screen != end) {
        QWindow* window = (*screen)->window();
        if (window) {
            if (window->geometry().contains(point)) return window;
        }
        screen++;
    }
    return nullptr;
}

QList<PlatformScreen::Mode> PlatformScreen::availableModes() const
{
    return m_availableModes;
}

ScreenPlatformWindow *PlatformScreen::primaryWindow() const
{
    return m_screenWindows.value(0, nullptr);
}

void PlatformScreen::addWindow(ScreenPlatformWindow *window)
{
    if (!window || m_screenWindows.contains(window)) return;
    DEBUG_MSG_SCREENS << "(screenWindow=" << window << ")";
    m_screenWindows.push_back(window);

    window->setGeometry(geometry());
    window->setActive(m_isActive);

    if (m_screenWindows.count() > 1) {
        DEBUG_MSG_SCREENS << "() - secondary window added to screen.";
    } else {
        primaryWindowChanged(m_screenWindows.at(0));
    }
}

void PlatformScreen::removeWindow(ScreenPlatformWindow *window)
{
    int index = m_screenWindows.indexOf(window);
    if (index >= 0) {
        DEBUG_MSG_SCREENS << "(screenWindow=" << window << ")";
        m_screenWindows.remove(index);
        if (index == 0) {
            Q_EMIT primaryWindowChanged(m_screenWindows.value(0, nullptr));
        }
    }
}

void PlatformScreen::setMirDisplayBuffer(mir::graphics::DisplayBuffer *buffer, mir::graphics::DisplaySyncGroup *group)
{
    DEBUG_MSG_SCREENS << "(renderTarget=" << as_render_target(buffer) << ", displayGroup=" << group << ")";
    // This operation should only be performed while rendering is stopped
    m_renderTarget = as_render_target(buffer);
    m_displayGroup = group;
}

void PlatformScreen::swapBuffers()
{
    m_renderTarget->swap_buffers();

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

void PlatformScreen::makeCurrent()
{
    m_renderTarget->make_current();
}

void PlatformScreen::doneCurrent()
{
    m_renderTarget->release_current();
}

bool PlatformScreen::internalDisplay() const
{
    using namespace mir::graphics;
    if (m_type == qtmir::OutputTypes::LVDS || m_type == qtmir::OutputTypes::EDP) {
        return true;
    }
    return false;
}
