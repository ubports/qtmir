/*
 * Copyright (C) 2013-2017 Canonical, Ltd.
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
#include <QMutexLocker>

#include "logging.h"

QMap<WId, ScreenWindow*> ScreenWindow::m_idToWindowMap;

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
    // Note: window->screen() is set to the primaryScreen(), if not specified explicitly.
    const auto myScreen = static_cast<Screen *>(window->screen()->handle());
    myScreen->setWindow(this);
    qCDebug(QTMIR_SCREENS) << "ScreenWindow" << this << "with window ID" << uint(m_winId) << "backed by" << myScreen << "with ID" << myScreen->outputId().as_value();

    QRect screenGeometry(screen()->availableGeometry());
    if (window->geometry() != screenGeometry) {
        setGeometry(screenGeometry);
        window->setGeometry(screenGeometry);
    }
    window->setSurfaceType(QSurface::OpenGLSurface);

    m_idToWindowMap[m_winId] = this;
}

ScreenWindow::~ScreenWindow()
{
    qCDebug(QTMIR_SCREENS) << "Destroying ScreenWindow" << this;

    m_idToWindowMap.remove(m_winId);
    static_cast<Screen *>(screen())->setWindow(nullptr);
}

ScreenWindow *ScreenWindow::findWithWId(WId id)
{
    return m_idToWindowMap.value(id, nullptr);
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

    // If backing a QQuickWindow, need to stop/start its renderer immediately
    auto quickWindow = static_cast<QQuickWindow *>(window());
    if (!quickWindow)
        return;

    auto renderer = QSGRenderLoop::instance();
    if (exposed) {
        renderer->show(quickWindow);
        QWindowSystemInterface::handleExposeEvent(window(), geometry()); // else it won't redraw
        QWindowSystemInterface::handleWindowActivated(window(), Qt::ActiveWindowFocusReason);
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

QRect ScreenWindow::availableDesktopArea() const
{
    QRect result;
    {
        QMutexLocker locker(&m_mutex);
        result = m_availableDesktopArea;
    }
    return result;
}

void ScreenWindow::setAvailableDesktopArea(const QRect &rect)
{
    QMutexLocker locker(&m_mutex);
    m_availableDesktopArea = rect;
}

QRect ScreenWindow::normalWindowMargins() const
{
    QRect result;
    {
        QMutexLocker locker(&m_mutex);
        result = m_normalWindowMargins;
    }
    return result;
}

void ScreenWindow::setNormalWindowMargins(const QRect &rect)
{
    QMutexLocker locker(&m_mutex);
    m_normalWindowMargins = rect;
}

QRect ScreenWindow::dialogWindowMargins() const
{
    QRect result;
    {
        QMutexLocker locker(&m_mutex);
        result = m_dialogWindowMargins;
    }
    return result;
}

void ScreenWindow::setDialogWindowMargins(const QRect &rect)
{
    QMutexLocker locker(&m_mutex);
    m_dialogWindowMargins = rect;
}
