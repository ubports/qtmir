/*
 * Copyright (C) 2013-2017 Canonical, Ltd.
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

#include "mirserverintegration.h"

#include <QtGlobal>

#include <QtFontDatabaseSupport/private/qgenericunixfontdatabase_p.h>
#include <QtEventDispatcherSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtServiceSupport/private/qgenericunixservices_p.h>

#include <qpa/qplatformwindow.h>
#include <qpa/qplatformaccessibility.h>
#include <qpa/qplatforminputcontext.h>
#include <qpa/qplatforminputcontextfactory_p.h>
#include <qpa/qwindowsysteminterface.h>

#include <QGuiApplication>
#include <QStringList>
#include <QDebug>

// Mir
#include <mir/graphics/display.h>
#include <mir/graphics/display_configuration.h>

// local
#include "clipboard.h"
#include "miropenglcontext.h"
#include "nativeinterface.h"
#include "offscreensurface.h"
#include "qmirserver.h"
#include "screen.h"
#include "screensmodel.h"
#include "screenwindow.h"
#include "services.h"
#include "ubuntutheme.h"
#include "logging.h"

namespace mg = mir::graphics;
using qtmir::Clipboard;

MirServerIntegration::MirServerIntegration()
    : m_accessibility(new QPlatformAccessibility())
    , m_fontDb(new QGenericUnixFontDatabase())
    , m_services(new Services)
    , m_mirServer(new QMirServer)
    , m_nativeInterface(nullptr)
{
    // For access to sensors, qtmir uses qtubuntu-sensors. qtubuntu-sensors reads the
    // UBUNTU_PLATFORM_API_BACKEND variable to decide if to load a valid sensor backend or not.
    // For it to function we need to ensure a valid backend has been specified
    if (qEnvironmentVariableIsEmpty("UBUNTU_PLATFORM_API_BACKEND")) {
        if (qgetenv("DESKTOP_SESSION").contains("mir") || !qEnvironmentVariableIsSet("ANDROID_DATA")) {
            qputenv("UBUNTU_PLATFORM_API_BACKEND", "desktop_mirclient");
        } else {
            qputenv("UBUNTU_PLATFORM_API_BACKEND", "touch_mirclient");
        }
    }

    // If Mir shuts down, quit.
    QObject::connect(m_mirServer.data(), &QMirServer::stopped,
                     QCoreApplication::instance(), &QCoreApplication::quit);

    m_inputContext = QPlatformInputContextFactory::create();

    // Default Qt behaviour doesn't match a shell's intentions, so customize:
    qGuiApp->setQuitOnLastWindowClosed(false);
}

MirServerIntegration::~MirServerIntegration()
{
    delete m_nativeInterface;
}

bool MirServerIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case ThreadedPixmaps: return true;
    case OpenGL: return true;
    case ThreadedOpenGL: return true;
    case BufferQueueingOpenGL: return true;
    case MultipleWindows: return true; // multi-monitor support
    case WindowManagement: return false; // platform has no WM, as this implements the WM!
    case NonFullScreenWindows: return false;
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPlatformWindow *MirServerIntegration::createPlatformWindow(QWindow *window) const
{
    QWindowSystemInterface::flushWindowSystemEvents();

    auto screens = m_mirServer->screensModel();
    if (!screens) {
        qCritical("Screens are not initialized, unable to create a new QWindow/ScreenWindow");
        return nullptr;
    }

    auto platformWindow = new ScreenWindow(window);
    if (screens->compositing()) {
        platformWindow->setExposed(true);
    }

    qCDebug(QTMIR_SCREENS) << "QWindow" << window << "with geom" << window->geometry()
                           << "is backed by a" << static_cast<Screen *>(window->screen()->handle())
                           << "with geometry" << window->screen()->geometry();
    return platformWindow;
}

QPlatformBackingStore *MirServerIntegration::createPlatformBackingStore(QWindow */*window*/) const
{
    return nullptr;
}

QPlatformOpenGLContext *MirServerIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
    return m_mirServer->createPlatformOpenGLContext(context);
}

QAbstractEventDispatcher *MirServerIntegration::createEventDispatcher() const
{
    return createUnixEventDispatcher();
}

void MirServerIntegration::handleScreenAdded(QPlatformScreen *screen)
{
    // TODO: remove this after we no longer support Qt < 5.13
    #if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
            this->screenAdded(screen);
    #else
            QWindowSystemInterface::handleScreenAdded(screen);
    #endif
}

void MirServerIntegration::handleScreenRemoved(QPlatformScreen *screen)
{
    // TODO: remove this after we no longer support Qt < 5.13
    #if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
            this->destroyScreen(screen);
    #else
            QWindowSystemInterface::handleScreenRemoved(screen);
    #endif
}

void MirServerIntegration::initialize()
{
    // Creates instance of and start the Mir server in a separate thread
    m_mirServer->start();

    auto screens = m_mirServer->screensModel();
    if (!screens) {
        qFatal("ScreensModel not initialized");
    }
    QObject::connect(screens.data(), &ScreensModel::screenAdded,
            [this](Screen *screen) { handleScreenAdded(screen); });
    QObject::connect(screens.data(), &ScreensModel::screenRemoved,
            [this](Screen *screen) {
        handleScreenRemoved(screen);
    });

    Q_FOREACH(auto screen, screens->screens()) {
        handleScreenAdded(screen);
    }

    m_nativeInterface = new NativeInterface(m_mirServer.data());
}

QPlatformAccessibility *MirServerIntegration::accessibility() const
{
    return m_accessibility.data();
}

QPlatformFontDatabase *MirServerIntegration::fontDatabase() const
{
    return m_fontDb.data();
}

QStringList MirServerIntegration::themeNames() const
{
    return QStringList(UbuntuTheme::name);
}

QPlatformTheme *MirServerIntegration::createPlatformTheme(const QString& name) const
{
    Q_UNUSED(name);
    return new UbuntuTheme;
}

QPlatformServices *MirServerIntegration::services() const
{
    return m_services.data();
}

QPlatformNativeInterface *MirServerIntegration::nativeInterface() const
{
    return m_nativeInterface;
}

#ifdef WITH_CONTENTHUB
QPlatformClipboard *MirServerIntegration::clipboard() const
{
    static QPlatformClipboard *clipboard = nullptr;
    if (!clipboard) {
        clipboard = new Clipboard;
    }
    return clipboard;
}
#endif

QPlatformOffscreenSurface *MirServerIntegration::createPlatformOffscreenSurface(
        QOffscreenSurface *surface) const
{
    return new OffscreenSurface(surface);
}
