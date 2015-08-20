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

#include "screenwindow.h"
#include "screen.h"

#include <mir/geometry/size.h>
#include <mir/graphics/display_buffer.h>

#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatformscreen.h>

#include "logging.h"

static WId newWId()
{
    static WId id = 0;

    if (id == std::numeric_limits<WId>::max())
        qWarning("MirServer QPA: Out of window IDs");

    return ++id;
}

ScreenWindow::ScreenWindow(QWindow *window)
    : QPlatformWindow(window)
    , m_winId(newWId())
{
    qCDebug(QTMIR_SCREENS) << "Window" << this << "with window ID" << uint(m_winId);

    // Register with the Screen it is associated with
    auto myScreen = static_cast<Screen *>(screen());
    Q_ASSERT(myScreen);
    myScreen->setWindow(this);

    QRect screenGeometry(screen()->availableGeometry());
    if (window->geometry() != screenGeometry) {
        setGeometry(screenGeometry);
    }
    window->setSurfaceType(QSurface::OpenGLSurface);

    // The compositor window is always active. I.e., it's always focused so that
    // it always processes key events, etc
    requestActivateWindow();
}

ScreenWindow::~ScreenWindow()
{
    static_cast<Screen *>(screen())->setWindow(nullptr);
}

void ScreenWindow::swapBuffers()
{
    static_cast<Screen *>(screen())->swapBuffers();
}

void ScreenWindow::makeCurrent()
{
    static_cast<Screen *>(screen())->makeCurrent();
}

void ScreenWindow::doneCurrent()
{
    static_cast<Screen *>(screen())->doneCurrent();
}
