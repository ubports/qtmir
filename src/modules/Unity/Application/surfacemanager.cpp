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
#include <workspacecontrollerinterface.h>

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
    m_workspaceController = static_cast<WorkspaceControllerInterface*>(nativeInterface->nativeResourceForIntegration("WorkspaceController"));

    auto windowModel = static_cast<WindowModelNotifier*>(nativeInterface->nativeResourceForIntegration("WindowModelNotifier"));
    connectToWindowModelNotifier(windowModel);
}

void SurfaceManager::connectToWindowModelNotifier(WindowModelNotifier *notifier)
{
    connect(notifier, &WindowModelNotifier::windowAdded,
            this, &SurfaceManager::onWindowAdded, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowRemoved,
            this, &SurfaceManager::onWindowRemoved, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowReady,
            this, [this](const miral::WindowInfo &windowInfo) {
        Q_EMIT surfaceReady(surfaceFor(windowInfo.window()));
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowMoved,
            this, [this](const miral::WindowInfo &windowInfo, const QPoint &top_left) {
        Q_EMIT surfaceMoved(surfaceFor(windowInfo.window()), top_left);
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowResized,
            this, [this](const miral::WindowInfo &windowInfo, const QSize &size) {
        Q_EMIT surfaceResized(surfaceFor(windowInfo.window()), size);
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowStateChanged,
            this, [this](const miral::WindowInfo &windowInfo, Mir::State state) {
        Q_EMIT surfaceStateChanged(surfaceFor(windowInfo.window()), state);
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowFocusChanged,
            this, [this](const miral::WindowInfo &windowInfo, bool focused) {
        Q_EMIT surfaceFocusChanged(surfaceFor(windowInfo.window()), focused);
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowRequestedRaise,
            this, [this](const miral::WindowInfo &windowInfo) {
        Q_EMIT surfaceRequestedRaise(surfaceFor(windowInfo.window()));
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowsRaised,
            this, [this](const std::vector<miral::Window> &windows) {
        Q_EMIT surfacesRaised(surfacesFor(windows));
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowsAddedToWorkspace,
            this, [this](const std::shared_ptr<miral::Workspace> &workspace, const std::vector<miral::Window> &windows) {
        Q_EMIT surfacesAddedToWorkspace(workspace, surfacesFor(windows));
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::windowsAboutToBeRemovedFromWorkspace,
            this, [this](const std::shared_ptr<miral::Workspace> &workspace, const std::vector<miral::Window> &windows) {
        Q_EMIT surfacesAboutToBeRemovedFromWorkspace(workspace, surfacesFor(windows));
    }, Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::modificationsEnded,   this, &SurfaceManager::modificationsEnded,    Qt::QueuedConnection);

    connect(notifier, &WindowModelNotifier::modificationsStarted, this, &SurfaceManager::modificationsStarted,  Qt::QueuedConnection);
}

void SurfaceManager::rememberMirSurface(MirSurface *surface)
{
    surface_to_window.insert({ surface, surface->window() });
}

void SurfaceManager::forgetMirSurface(const miral::Window &window)
{
    std::shared_ptr<mir::scene::Surface> msSurface = window;
    surface_to_window.right.erase(window);
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
    SessionInterface* session = ApplicationManager::singleton()->findSession(mirSession.get());

    MirSurface *parentSurface = surfaceFor(window.windowInfo.parent());

    auto surface = new MirSurface(window, m_windowController, session, parentSurface);
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
    MirSurface *surface = surfaceFor(windowInfo.window());
    if (!surface) return;

    forgetMirSurface(windowInfo.window());
    tracepoint(qtmir, surfaceDestroyed);

    Q_EMIT surfaceRemoved(surface);
}

MirSurface *SurfaceManager::surfaceFor(const miral::Window &window) const
{
    auto window_iterator = surface_to_window.right.find(window);
    if(window_iterator != surface_to_window.right.end()) {
        return window_iterator->second;
    }
    return nullptr;
}

QVector<unity::shell::application::MirSurfaceInterface *> SurfaceManager::surfacesFor(const std::vector<miral::Window> &windows) const
{
    QVector<unityapi::MirSurfaceInterface*> surfaces;
    for (size_t i = 0; i < windows.size(); i++) {
        auto mirSurface = surfaceFor(windows[i]);
        if (mirSurface) {
            surfaces.push_back(mirSurface);
        } else {
            std::shared_ptr<mir::scene::Surface> ms = windows[i];
            WARNING_MSG << " Could not find qml surface for " << ms.get();
        }
    }
    return surfaces;
}

miral::Window SurfaceManager::windowFor(MirSurface *surface) const
{
    auto window_iterator = surface_to_window.left.find(surface);
    if(window_iterator != surface_to_window.left.end()) {
        return window_iterator->second;
    }
    return miral::Window();
}

void SurfaceManager::raise(unityapi::MirSurfaceInterface *surface)
{
    DEBUG_MSG << "(" << surface << ")";
    auto qtmirSurface = static_cast<qtmir::MirSurface*>(surface);
    m_windowController->raise(qtmirSurface->window());
}

void SurfaceManager::activate(unityapi::MirSurfaceInterface *surface)
{
    auto qtmirSurface = static_cast<MirSurface*>(surface);
    m_windowController->activate(qtmirSurface ? qtmirSurface->window() : miral::Window());
}

void SurfaceManager::forEachSurfaceInWorkspace(const std::shared_ptr<miral::Workspace> &workspace,
                                               const std::function<void(unity::shell::application::MirSurfaceInterface *)> &callback)
{
    m_workspaceController->forEachWindowInWorkspace(workspace, [&](const miral::Window &window) {
        auto surface = surfaceFor(window);
        if (surface) {
            callback(surface);
        }
    });
}

void SurfaceManager::moveSurfaceToWorkspace(unity::shell::application::MirSurfaceInterface *surface,
                                            const std::shared_ptr<miral::Workspace> &workspace)
{
    miral::Window window = windowFor(static_cast<qtmir::MirSurface*>(surface));
    if (window) {
        m_workspaceController->moveWindowToWorkspace(window, workspace);
    }
}

void SurfaceManager::moveWorkspaceContentToWorkspace(const std::shared_ptr<miral::Workspace> &to,
                                                     const std::shared_ptr<miral::Workspace> &from)
{
    m_workspaceController->moveWorkspaceContentToWorkspace(to, from);
}
