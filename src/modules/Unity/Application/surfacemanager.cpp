/*
 * Copyright (C) 2016 Canonical, Ltd.
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
#include "sessionmanager.h"
#include "tracepoints.h"

// mirserver
#include "nativeinterface.h"

// common
#include <debughelpers.h>
#include <mirqtconversion.h>

// Mir
#include <mir/scene/surface.h>

// Qt
#include <QGuiApplication>

Q_LOGGING_CATEGORY(QTMIR_SURFACEMANAGER, "qtmir.surfacemanager", QtInfoMsg)

#define DEBUG_MSG qCDebug(QTMIR_SURFACEMANAGER).nospace().noquote() << __func__
#define WARNING_MSG qCWarning(QTMIR_SURFACEMANAGER).nospace().noquote() << __func__

using namespace qtmir;
namespace unityapi = unity::shell::application;

SurfaceManager *SurfaceManager::instance()
{
    static SurfaceManager* instance{nullptr};
    if (!instance) {
        instance = new SurfaceManager;
    }
    return instance;
}

SurfaceManager::SurfaceManager(QObject *)
{
    DEBUG_MSG << "()";

    auto nativeInterface = dynamic_cast<NativeInterface*>(QGuiApplication::platformNativeInterface());

    if (!nativeInterface) {
        qFatal("ERROR: Unity.Application QML plugin requires use of the 'mirserver' QPA plugin");
    }

    m_windowController = static_cast<WindowControllerInterface*>(nativeInterface->nativeResourceForIntegration("WindowController"));

    auto windowModel = static_cast<WindowModelNotifier*>(nativeInterface->nativeResourceForIntegration("WindowModelNotifier"));
    connectToWindowModelNotifier(windowModel);

    m_sessionManager = SessionManager::singleton();
}

void SurfaceManager::connectToWindowModelNotifier(WindowModelNotifier *notifier)
{
    connect(notifier, &WindowModelNotifier::windowAdded,
            this, &SurfaceManager::onWindowAdded, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowRemoved,
            this, &SurfaceManager::onWindowRemoved, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowReady,
            this, [this](const miral::WindowInfo &windowInfo) {
        Q_EMIT surfaceReady(find(windowInfo.window()));
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowMoved,
            this, [this](const miral::WindowInfo &windowInfo, const QPoint &top_left) {
        Q_EMIT surfaceMoved(find(windowInfo.window()), top_left);
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowResized,
            this, [this](const miral::WindowInfo &windowInfo, const QSize &size) {
        Q_EMIT surfaceResized(find(windowInfo.window()), size);
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowStateChanged,
            this, [this](const miral::WindowInfo &windowInfo, Mir::State state) {
        Q_EMIT surfaceStateChanged(find(windowInfo.window()), state);
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowFocusChanged,
            this, [this](const miral::WindowInfo &windowInfo, bool focused) {
        Q_EMIT surfaceFocusChanged(find(windowInfo.window()), focused);
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowRequestedRaise,
            this, [this](const miral::WindowInfo &windowInfo) {
        Q_EMIT surfaceRequestedRaise(find(windowInfo.window()));
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowsRaised,
            this, [this](const std::vector<miral::Window> &windows) {
        Q_EMIT surfacesRaised(find(windows));
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowsAddedToWorkspace,
            this, [this](const std::shared_ptr<miral::Workspace> &workspace, const std::vector<miral::Window> &windows) {
        Q_EMIT surfacesAddedToWorkspace(workspace, find(windows));
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowsAboutToBeRemovedFromWorkspace,
            this, [this](const std::shared_ptr<miral::Workspace> &workspace, const std::vector<miral::Window> &windows) {
        Q_EMIT surfacesAboutToBeRemovedFromWorkspace(workspace, find(windows));
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::modificationsEnded,   this, &SurfaceManager::modificationsEnded,    Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::modificationsStarted, this, &SurfaceManager::modificationsStarted,  Qt::QueuedConnection);
}

unity::shell::application::MirSurfaceInterface *SurfaceManager::surfaceFor(const miral::Window &window) const
{
    return find(window);
}

void SurfaceManager::rememberMirSurface(MirSurface *surface)
{
    std::shared_ptr<mir::scene::Surface> msSurface = surface->window();
    m_allSurfaces.insert((qintptr)msSurface.get(), surface);
}

void SurfaceManager::forgetMirSurface(const miral::Window &window)
{
    std::shared_ptr<mir::scene::Surface> msSurface = window;
    m_allSurfaces.remove((qintptr)msSurface.get());
}

void SurfaceManager::onWindowAdded(const NewWindow &window)
{
    {
        std::shared_ptr<mir::scene::Surface> surface = window.surface;
        DEBUG_MSG << " mir::scene::Surface[type=" << mirSurfaceTypeToStr(surface->type())
            << ",parent=" << (void*)(surface->parent().get())
            << ",state=" << mirSurfaceStateToStr(surface->state())
            << ",top_left=" << toQPoint(surface->top_left())
            << "]";
    }

    auto mirSession = window.windowInfo.window().application();
    SessionInterface* session = m_sessionManager->findSession(mirSession.get());

    MirSurface *parentSurface;
    {
        std::shared_ptr<mir::scene::Surface> surface = window.windowInfo.window();
        parentSurface = find(surface->parent());
    }

    auto surface = new MirSurface(window.windowInfo, m_windowController, session, parentSurface);
    rememberMirSurface(surface);

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
    MirSurface *surface = find(windowInfo.window());
    if (!surface) return;

    forgetMirSurface(windowInfo.window());
    tracepoint(qtmir, surfaceDestroyed);

    Q_EMIT surfaceRemoved(surface);
}

MirSurface *SurfaceManager::find(const miral::Window &window) const
{
    std::shared_ptr<mir::scene::Surface> msSurface = window;
    auto iter = m_allSurfaces.find((qintptr)msSurface.get());
    if (iter != m_allSurfaces.end()) {
        return *iter;
    }
    return nullptr;
}

MirSurface *SurfaceManager::find(const std::shared_ptr<mir::scene::Surface> &msSurface) const
{
    auto iter = m_allSurfaces.find((qintptr)msSurface.get());
    if (iter != m_allSurfaces.end()) {
        return *iter;
    }
    return nullptr;
}

QVector<unity::shell::application::MirSurfaceInterface *> SurfaceManager::find(const std::vector<miral::Window> &windows) const
{
    QVector<unityapi::MirSurfaceInterface*> surfaces;
    for (size_t i = 0; i < windows.size(); i++) {
        auto mirSurface = find(windows[i]);
        if (mirSurface) {
            surfaces.push_back(mirSurface);
        } else {
            WARNING_MSG << " Could not find qml surface for " << windows[i];
        }
    }
    return surfaces;
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
