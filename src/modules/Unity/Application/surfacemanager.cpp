/*
 * Copyright (C) 2016,2017 Canonical, Ltd.
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

#include "surfacemanager.h"

#include "mirsurface.h"
#include "application_manager.h"
#include "tracepoints.h"

// mirserver
#include "nativeinterface.h"

// common
#include <debughelpers.h>
#include <mirqtconversion.h>

// Qt
#include <QGuiApplication>

Q_LOGGING_CATEGORY(QTMIR_SURFACEMANAGER, "qtmir.surfacemanager", QtInfoMsg)

#define DEBUG_MSG qCDebug(QTMIR_SURFACEMANAGER).nospace().noquote() << __func__
#define WARNING_MSG qCWarning(QTMIR_SURFACEMANAGER).nospace().noquote() << __func__

using namespace qtmir;
namespace unityapi = unity::shell::application;


SurfaceManager::SurfaceManager()
{
    DEBUG_MSG << "()";

    auto nativeInterface = dynamic_cast<NativeInterface*>(QGuiApplication::platformNativeInterface());

    if (!nativeInterface) {
        qFatal("ERROR: Unity.Application QML plugin requires use of the 'mirserver' QPA plugin");
    }

    m_windowController = static_cast<WindowControllerInterface*>(nativeInterface->nativeResourceForIntegration("WindowController"));

    auto windowModel = static_cast<WindowModelNotifier*>(nativeInterface->nativeResourceForIntegration("WindowModelNotifier"));
    connectToWindowModelNotifier(windowModel);
}

SurfaceManager::SurfaceManager(WindowControllerInterface *windowController,
                               WindowModelNotifier *windowModel,
                               SessionManager *sessionManager)
    : m_windowController(windowController)
    , m_sessionManager(sessionManager)
{
    DEBUG_MSG << "()";
    connectToWindowModelNotifier(windowModel);
}

void SurfaceManager::connectToWindowModelNotifier(WindowModelNotifier *notifier)
{
    connect(notifier, &WindowModelNotifier::windowAdded,          this, &SurfaceManager::onWindowAdded,           Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowRemoved,        this, &SurfaceManager::onWindowRemoved,         Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowReady,          this, &SurfaceManager::onWindowReady,           Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowMoved,          this, &SurfaceManager::onWindowMoved,           Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowStateChanged,   this, &SurfaceManager::onWindowStateChanged,    Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowFocusChanged,   this, &SurfaceManager::onWindowFocusChanged,    Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowsRaised,        this, &SurfaceManager::onWindowsRaised,         Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowRequestedRaise, this, &SurfaceManager::onWindowsRequestedRaise, Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::modificationsStarted, this, &SurfaceManager::modificationsStarted,    Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::modificationsEnded,   this, &SurfaceManager::modificationsEnded,      Qt::QueuedConnection);
}

void SurfaceManager::rememberMirSurface(MirSurface *surface)
{
    m_allSurfaces.append(surface);
}

void SurfaceManager::forgetMirSurface(const miral::Window &window)
{
    for (int i = 0; i < m_allSurfaces.count(); ++i) {
        if (m_allSurfaces[i]->window() == window) {
            m_allSurfaces.removeAt(i);
            return;
        }
    }
}

void SurfaceManager::onWindowAdded(const NewWindow &window)
{
    const auto &windowInfo = window.windowInfo;
    {
        DEBUG_MSG << " mir::scene::Surface[type=" << mirSurfaceTypeToStr(windowInfo.type())
            << ",parent=" << (void*)(std::shared_ptr<mir::scene::Surface>{windowInfo.parent()}.get())
            << ",state=" << mirSurfaceStateToStr(windowInfo.state())
            << ",top_left=" << toQPoint(windowInfo.window().top_left())
            << "]";
    }

    auto mirSession = windowInfo.window().application();
    SessionInterface* session = ApplicationManager::singleton()->findSession(mirSession.get());

    const auto parentSurface = find(windowInfo.parent());
    const auto surface = new MirSurface(window, m_windowController, session, parentSurface);
    rememberMirSurface(surface);

    connect(surface, &MirSurface::isBeingDisplayedChanged, this, [this](MirSurfaceInterface *surface) {
        if ((!surface->live() || !surface->session())
                && !surface->isBeingDisplayed()) {
            forgetMirSurface(static_cast<MirSurface*>(surface)->window());
            delete surface;
            tracepoint(qtmir, surfaceDestroyed);
        }
    });

    if (parentSurface) {
        static_cast<MirSurfaceListModel*>(parentSurface->childSurfaceList())->prependSurface(surface);
    }

    if (session)
        session->registerSurface(surface);

    tracepoint(qtmir, surfaceCreated);
    Q_EMIT surfaceCreated(surface);
}

void SurfaceManager::onWindowRemoved(const miral::WindowInfo &windowInfo)
{
    DEBUG_MSG << "()";
    MirSurface *surface = find(windowInfo);
    forgetMirSurface(windowInfo.window());
    if (surface->isBeingDisplayed()) {
        surface->setLive(false);
    } else {
        delete surface;
        tracepoint(qtmir, surfaceDestroyed);
    }
}

MirSurface *SurfaceManager::find(const miral::WindowInfo &needle) const
{
    return find(needle.window());
}

MirSurface *SurfaceManager::find(const miral::Window &window) const
{
    Q_FOREACH(const auto surface, m_allSurfaces) {
        if (surface->window() == window) {
            return surface;
        }
    }
    return nullptr;
}

void SurfaceManager::onWindowReady(const miral::WindowInfo &windowInfo)
{
    if (auto mirSurface = find(windowInfo)) {
        tracepoint(qtmir, firstFrameDrawn); // MirAL decides surface ready when it swaps its first frame
        mirSurface->setReady();
    }
}

void SurfaceManager::onWindowMoved(const miral::WindowInfo &windowInfo, const QPoint topLeft)
{
    if (auto mirSurface = find(windowInfo)) {
        mirSurface->setPosition(topLeft);
    }
}

void SurfaceManager::onWindowFocusChanged(const miral::WindowInfo &windowInfo, bool focused)
{
    if (auto mirSurface = find(windowInfo)) {
        mirSurface->setFocused(focused);
    }
}

void SurfaceManager::onWindowStateChanged(const miral::WindowInfo &windowInfo, Mir::State state)
{
    if (auto mirSurface = find(windowInfo)) {
        mirSurface->updateState(state);
    }
}

void SurfaceManager::onWindowsRaised(const std::vector<miral::Window> &windows)
{
    // sad inefficiency when crossing API boundaries (from miral to qt)

    const int raiseCount = windows.size();

    DEBUG_MSG << "() raiseCount = " << raiseCount;

    QVector<unityapi::MirSurfaceInterface*> surfaces(raiseCount);
    for (int i = 0; i < raiseCount; i++) {
        auto mirSurface = find(windows[i]);
        if (mirSurface) {
            surfaces[i] = mirSurface;
        } else {
            WARNING_MSG << " Could not find qml surface for " << windows[i];
        }
    }
    Q_EMIT surfacesRaised(surfaces);
}

void SurfaceManager::onWindowsRequestedRaise(const miral::WindowInfo &windowInfo)
{
    if (auto mirSurface = find(windowInfo)) {
        mirSurface->requestFocus();
    }
}

void SurfaceManager::raise(unityapi::MirSurfaceInterface *surface)
{
    DEBUG_MSG << "(" << surface << ")";
    auto qtmirSurface = static_cast<qtmir::MirSurface*>(surface);
    m_windowController->raise(qtmirSurface->window());
}

void SurfaceManager::activate(unityapi::MirSurfaceInterface *surface)
{
    auto qtmirSurface = static_cast<qtmir::MirSurface*>(surface);
    m_windowController->activate(qtmirSurface ? qtmirSurface->window() : miral::Window());
}
