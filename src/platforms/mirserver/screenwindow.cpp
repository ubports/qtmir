/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
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

#define DEBUG_MSG qCDebug(QTMIR_SCREENS).nospace() << "ScreenWindow[" << this <<"]::" << __func__

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
    const auto platformScreen = static_cast<Screen *>(screen());
    QRect screenGeometry(platformScreen->availableGeometry());

    // Note: screen() is set to the primaryScreen(), if not specified explicitly.
    DEBUG_MSG << "(window=" << window
              << ") - windowId=" << uint(m_winId)
              << ", on screen=" << platformScreen
              << ", outputId=" << platformScreen->outputId().as_value()
              << ", geometry=" << screenGeometry;

    platformScreen->setWindow(this);
    window->setSurfaceType(QSurface::OpenGLSurface);
}

ScreenWindow::~ScreenWindow()
{
    DEBUG_MSG << "()";
    static_cast<Screen *>(screen())->setWindow(nullptr);
}

void ScreenWindow::setGeometry(const QRect &rect)
{
    DEBUG_MSG << "(" << rect << ")";
    QWindowSystemInterface::handleGeometryChange(window(), rect);
    QPlatformWindow::setGeometry(rect);
}

bool ScreenWindow::isExposed() const
{
    return m_exposed;
}

void ScreenWindow::setExposed(const bool exposed)
{
    DEBUG_MSG << "(exposed=" << (exposed ? "true" : "false") << ")";
    if (m_exposed == exposed)
        return;

    m_exposed = exposed;
    if (!window())
        return;

    // If backing a QQuickWindow, need to stop/start its renderer immediately
    auto quickWindow = static_cast<QQuickWindow *>(window());
    if (!quickWindow)
        return;

    auto renderer = QSGRenderLoop::instance();
    if (exposed) {
        renderer->show(quickWindow);
        QWindowSystemInterface::handleExposeEvent(window(), geometry()); // else it won't redraw
    } else {
        quickWindow->setPersistentOpenGLContext(false);
        quickWindow->setPersistentSceneGraph(false);
        renderer->hide(quickWindow); // ExposeEvent will arrive too late, need to stop compositor immediately
    }
}

void ScreenWindow::setScreen(QPlatformScreen *newScreen)
{
    auto platformScreen = static_cast<Screen *>(newScreen);
    Q_ASSERT(platformScreen);
    DEBUG_MSG << "(screen=" << platformScreen << ", outputId=" << platformScreen->outputId().as_value() << ")";

    // Dis-associate the old screen
    if (screen()) {
        static_cast<Screen *>(screen())->setWindow(nullptr);
    }
    // Associate new screen and announce to Qt
    platformScreen->setWindow(this);

    QWindowSystemInterface::handleWindowScreenChanged(window(), platformScreen->screen());
    setExposed(true); //GERRY - assumption setScreen only called while compositor running
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
