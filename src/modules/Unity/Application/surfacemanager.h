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

Q_DECLARE_LOGGING_CATEGORY(QTMIR_SURFACEMANAGER)

namespace qtmir {

class MirSurface;
class WindowControllerInterface;

class SurfaceManager : public unity::shell::application::SurfaceManagerInterface
{
    Q_OBJECT

public:
    explicit SurfaceManager(QObject *parent = 0);
    virtual ~SurfaceManager() {}

    void raise(unity::shell::application::MirSurfaceInterface *surface) override;
    void activate(unity::shell::application::MirSurfaceInterface *surface) override;

protected:
    // for testing purposes
    SurfaceManager(WindowControllerInterface *windowController,
                   WindowModelNotifier *windowModel,
                   SessionManager *sessionManager);

private Q_SLOTS:
    void onWindowAdded(const qtmir::NewWindow &windowInfo);
    void onWindowRemoved(const miral::WindowInfo &windowInfo);
    void onWindowReady(const miral::WindowInfo &windowInfo);
    void onWindowMoved(const miral::WindowInfo &windowInfo, const QPoint topLeft);
    void onWindowStateChanged(const miral::WindowInfo &windowInfo, Mir::State state);
    void onWindowFocusChanged(const miral::WindowInfo &windowInfo, bool focused);
    void onWindowsRaised(const std::vector<miral::Window> &windows);
    void onWindowsRequestedRaise(const miral::WindowInfo &windowInfo);

private:
    void connectToWindowModelNotifier(WindowModelNotifier *notifier);
    void rememberMirSurface(MirSurface *surface);
    void forgetMirSurface(const miral::Window &window);
    MirSurface* find(const miral::WindowInfo &needle) const;
    MirSurface* find(const miral::Window &needle) const;

    QVector<MirSurface*> m_allSurfaces;

    WindowControllerInterface *m_windowController;
};

} // namespace qtmir

#endif // QTMIR_SURFACEMANAGER_H
