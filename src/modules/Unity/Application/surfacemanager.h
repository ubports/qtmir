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

#ifndef QTMIR_SURFACEMANAGER_H
#define QTMIR_SURFACEMANAGER_H

// qtmir
#include "qtmir/windowmodelnotifier.h"

// Unity API
#include <unity/shell/application/SurfaceManagerInterface.h>

#include <QVector>
#include <QLoggingCategory>

#include <boost/bimap.hpp>

Q_DECLARE_LOGGING_CATEGORY(QTMIR_SURFACEMANAGER)

namespace qtmir {

class MirSurface;
class SessionManager;
class WindowControllerInterface;
class WorkspaceControllerInterface;

class SurfaceManager : public unity::shell::application::SurfaceManagerInterface
{
    Q_OBJECT
public:
    virtual ~SurfaceManager() = default;

    static SurfaceManager *instance();

    void raise(unity::shell::application::MirSurfaceInterface *surface) override;
    void activate(unity::shell::application::MirSurfaceInterface *surface) override;

    void forEachSurfaceInWorkspace(const std::shared_ptr<miral::Workspace> &workspace,
                                   const std::function<void(unity::shell::application::MirSurfaceInterface*)> &callback) override;
    void moveSurfaceToWorkspace(unity::shell::application::MirSurfaceInterface* surface,
                                const std::shared_ptr<miral::Workspace> &workspace) override;
    void moveWorkspaceContentToWorkspace(const std::shared_ptr<miral::Workspace> &to,
                                         const std::shared_ptr<miral::Workspace> &from) override;

private Q_SLOTS:
    void onWindowAdded(const qtmir::NewWindow &windowInfo);
    void onWindowRemoved(const miral::WindowInfo &windowInfo);
private:
    explicit SurfaceManager(QObject *parent = 0);

    void connectToWindowModelNotifier(WindowModelNotifier *notifier);
    void rememberMirSurface(MirSurface *surface);
    void forgetMirSurface(const miral::Window &window);
    MirSurface* surfaceFor(const miral::Window &window) const;
    QVector<unity::shell::application::MirSurfaceInterface*> surfacesFor(const std::vector<miral::Window> &windows) const;
    miral::Window windowFor(MirSurface *surface) const;

    WindowControllerInterface *m_windowController;
    WorkspaceControllerInterface *m_workspaceController;
    SessionManager* m_sessionManager;

    friend class Workspace;
    using swbimap_t = boost::bimap<MirSurface*, miral::Window>;
    swbimap_t surface_to_window;
};

} // namespace qtmir

Q_DECLARE_METATYPE(std::shared_ptr<miral::Workspace>)

#endif // QTMIR_SURFACEMANAGER_H
