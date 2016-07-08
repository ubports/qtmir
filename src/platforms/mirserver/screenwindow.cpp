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

#define DEBUG_MSG qCDebug(QTMIR_SCREENS).nospace() << "ScreenWindow[" << (void*)this <<"]::" << __func__

static WId newWId()
{
    static WId id = 0;

    if (id == std::numeric_limits<WId>::max())
        qWarning("MirServer QPA: Out of window IDs");

    return ++id;
}

ScreenWindow::ScreenWindow(QWindow *window)
    : QObject(nullptr)
    , QPlatformWindow(window)
    , m_exposed(false)
    , m_primary(false)
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

    platformScreen->addWindow(this);

    m_primary = platformScreen->primaryWindow() == this;
    connect(platformScreen, &Screen::primaryWindowChanged, this, [this](ScreenWindow* primaryWindow) {
        setPrimary(primaryWindow == this);
    });

    window->setSurfaceType(QSurface::OpenGLSurface);
}

ScreenWindow::~ScreenWindow()
{
    DEBUG_MSG << "()";
    disconnect(static_cast<Screen *>(screen()), &Screen::primaryWindowChanged, this, 0);
    static_cast<Screen *>(screen())->removeWindow(this);
}

void ScreenWindow::setGeometry(const QRect &rect)
{
    DEBUG_MSG << "(" << rect << ")";
    QWindowSystemInterface::handleGeometryChange(window(), rect);
    QPlatformWindow::setGeometry(rect);
}

bool ScreenWindow::isExposed() const
{
    return m_exposed && m_primary;
}

void ScreenWindow::setExposed(const bool exposed)
{
    if (m_exposed == exposed)
        return;

    m_exposed = exposed;
    updateExpose();
}

void ScreenWindow::setPrimary(const bool primary)
{
    if (m_primary == primary)
        return;

    m_primary = primary;
    updateExpose();
}

void ScreenWindow::updateExpose()
{
    if (!window())
        return;
    DEBUG_MSG << "(exposed=" << (m_exposed ? "true" : "false") << ", primary=" << (m_primary ? "true" : "false") << ")";

    // If backing a QQuickWindow, need to stop/start its renderer immediately
    auto quickWindow = static_cast<QQuickWindow *>(window());
    if (!quickWindow)
        return;

    auto renderer = QSGRenderLoop::instance();
    if (isExposed()) {
        renderer->show(quickWindow);
        QWindowSystemInterface::handleExposeEvent(window(), geometry()); // else it won't redraw
    } else {
        // set to non-persistent so we re-create when exposed. // NICK - why?
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
        disconnect(static_cast<Screen *>(screen()), &Screen::primaryWindowChanged, this, 0);
        static_cast<Screen *>(screen())->removeWindow(this);
    }
    // Associate new screen and announce to Qt
    platformScreen->addWindow(this);
    setPrimary(platformScreen->primaryWindow() == this);

    connect(platformScreen, &Screen::primaryWindowChanged, this, [this](ScreenWindow* primaryWindow) {
        setPrimary(primaryWindow == this);
    });

    QWindowSystemInterface::handleWindowScreenChanged(window(), platformScreen->screen());
}

void ScreenWindow::swapBuffers()
{
    qDebug() << "swap buffers" << this;

    auto scrn = static_cast<Screen *>(screen());
    if (scrn) scrn->swapBuffers();
}

void ScreenWindow::makeCurrent()
{
    auto scrn = static_cast<Screen *>(screen());
    if (scrn) scrn->makeCurrent();
}

void ScreenWindow::doneCurrent()
{
    auto scrn = static_cast<Screen *>(screen());
    if (scrn) scrn->doneCurrent();
}
