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

// Mir
#include <mir/geometry/size.h>
#include <mir/graphics/display_buffer.h>

// Qt
#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatformscreen.h>
#include <QQuickWindow>
#include <QtQuick/private/qsgrenderloop_p.h>
#include <QDebug>

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
    , m_exposed(false)
    , m_winId(newWId())
{
    // Register with the Screen it is associated with
    auto myScreen = static_cast<Screen *>(screen());
    Q_ASSERT(myScreen);
    myScreen->setWindow(this);

    qCDebug(QTMIR_SCREENS) << "ScreenWindow" << this << "with window ID" << uint(m_winId) << "backed by" << myScreen;

    QRect screenGeometry(screen()->availableGeometry());
    if (window->geometry() != screenGeometry) {
        setGeometry(screenGeometry);
        window->setGeometry(screenGeometry);
    }
    window->setSurfaceType(QSurface::OpenGLSurface);

    // The compositor window is always active. I.e., it's always focused so that
    // it always processes key events, etc
    requestActivateWindow();
}

ScreenWindow::~ScreenWindow()
{
    qCDebug(QTMIR_SCREENS) << "Destroying ScreenWindow" << this;
    static_cast<Screen *>(screen())->setWindow(nullptr);
}

bool ScreenWindow::isExposed() const
{
    return m_exposed;
}

void ScreenWindow::setExposed(const bool exposed)
{
    qCDebug(QTMIR_SCREENS) << "ScreenWindow::setExposed" << this << exposed;
    if (m_exposed == exposed)
        return;

    m_exposed = exposed;
    if (!window())
        return;

    // If backing a QQuickWindow, need to stop/start its renderer immediately
    auto quickWindow = static_cast<QQuickWindow *>(window());
    if (!quickWindow)
        return;

    if (exposed) {
        QWindowSystemInterface::handleExposeEvent(window(), QRegion());
    } else {
        auto renderer = QSGRenderLoop::instance();
        renderer->hide(quickWindow); // ExposeEvent will arrive too late, need to stop compositor immediately
    }
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
