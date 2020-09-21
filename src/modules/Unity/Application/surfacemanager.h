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

#ifndef QTMIR_SURFACEMANAGER_H
#define QTMIR_SURFACEMANAGER_H

// common
#include "windowmodelnotifier.h"

// Unity API
#include <unity/shell/application/SurfaceManagerInterface.h>

#include <QVector>
#include <QLoggingCategory>

#include <boost/bimap.hpp>

Q_DECLARE_LOGGING_CATEGORY(QTMIR_SURFACEMANAGER)

namespace qtmir {

class MirSurface;
class SessionMapInterface;
class WindowControllerInterface;
class WorkspaceControllerInterface;

class SurfaceManager : public unity::shell::application::SurfaceManagerInterface
{
    Q_OBJECT

public:
    SurfaceManager(WindowControllerInterface *windowController,
                   WindowModelNotifier *windowModel,
                   SessionMapInterface *sessionMap);
    virtual ~SurfaceManager() {}

    static SurfaceManager *instance();

    void raise(unity::shell::application::MirSurfaceInterface *surface) override;
    void activate(unity::shell::application::MirSurfaceInterface *surface) override;

    void forEachSurfaceInWorkspace(const std::shared_ptr<miral::Workspace> &workspace,
                                   const std::function<void(unity::shell::application::MirSurfaceInterface*)> &callback) override;
    void moveSurfaceToWorkspace(unity::shell::application::MirSurfaceInterface* surface,
                                const std::shared_ptr<miral::Workspace> &workspace) override;
    void moveWorkspaceContentToWorkspace(const std::shared_ptr<miral::Workspace> &to,
                                         const std::shared_ptr<miral::Workspace> &from) override;

    // mainly for test usage
    MirSurface* surfaceFor(const miral::Window &window) const;

private Q_SLOTS:
    void onWindowAdded(const qtmir::NewWindow &windowInfo);
    void onWindowRemoved(const miral::WindowInfo &windowInfo);
private:
    explicit SurfaceManager();

    void connectToWindowModelNotifier(WindowModelNotifier *notifier);
    void rememberMirSurface(MirSurface *surface);
    void forgetMirSurface(const miral::Window &window);
    QVector<unity::shell::application::MirSurfaceInterface*> surfacesFor(const std::vector<miral::Window> &windows) const;
    miral::Window windowFor(MirSurface *surface) const;

    WindowControllerInterface *m_windowController;
    WorkspaceControllerInterface *m_workspaceController;
    SessionMapInterface *m_sessionMap;

    friend class Workspace;
    using swbimap_t = boost::bimap<MirSurface*, miral::Window>;
    swbimap_t surface_to_window;
};

} // namespace qtmir

Q_DECLARE_METATYPE(std::shared_ptr<miral::Workspace>)

#endif // QTMIR_SURFACEMANAGER_H
