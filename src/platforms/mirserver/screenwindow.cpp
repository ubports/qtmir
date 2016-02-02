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
    qCDebug(QTMIR_SCREENS) << "ScreenWindow" << this << "with window ID" << uint(m_winId) << "UNBACKED!";

    // Note: window->screen() is set to the primaryScreen(), if not specified explicitly.
    QRect screenGeometry(screen()->availableGeometry());
    if (window->geometry() != screenGeometry) {
        setGeometry(screenGeometry);
        window->setGeometry(screenGeometry);
    }
    window->setSurfaceType(QSurface::OpenGLSurface);
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
    qCDebug(QTMIR_SCREENS) << "ScreenWindow::setExposed" << this << exposed << screen();
    if (m_exposed == exposed)
        return;

    m_exposed = exposed;
    if (!window())
        return;

    if (exposed) {
        // Register with the Screen it is associated with when marked visible.
        auto myScreen = static_cast<Screen *>(window()->screen()->handle());
        myScreen->setWindow(this);
        qCDebug(QTMIR_SCREENS) << "ScreenWindow" << this << "with window ID" << uint(m_winId) << "now backed by" << myScreen << "with ID" << myScreen->outputId().as_value();

        QRect screenGeometry(screen()->availableGeometry());
        if (window()->geometry() != screenGeometry) {
            setGeometry(screenGeometry);
            window()->setGeometry(screenGeometry);
        }
    }

    // If backing a QQuickWindow, need to stop/start its renderer immediately
    auto quickWindow = static_cast<QQuickWindow *>(window());
    if (!quickWindow)
        return;

    auto renderer = QSGRenderLoop::instance();
    if (exposed) {
        renderer->show(quickWindow);
        QWindowSystemInterface::handleExposeEvent(window(), QRegion()); // else it won't redraw
    } else {
        quickWindow->setPersistentOpenGLContext(false);
        quickWindow->setPersistentSceneGraph(false);
        renderer->hide(quickWindow); // ExposeEvent will arrive too late, need to stop compositor immediately
    }
}

void ScreenWindow::setScreen(QPlatformScreen *newScreen)
{
    // Dis-associate the old screen
    if (screen()) {
        static_cast<Screen *>(screen())->setWindow(nullptr);
    }

    // Associate new screen and announce to Qt
    auto myScreen = static_cast<Screen *>(newScreen);
    Q_ASSERT(myScreen);
    myScreen->setWindow(this);

    QWindowSystemInterface::handleWindowScreenChanged(window(), myScreen->screen());
    setExposed(true); //GERRY - assumption setScreen only called while compositor running

    qCDebug(QTMIR_SCREENS) << "ScreenWindow" << this << "with window ID" << uint(m_winId) << "NEWLY backed by" << myScreen;
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
