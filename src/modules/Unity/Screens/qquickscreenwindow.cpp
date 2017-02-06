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

#include "qquickscreenwindow.h"

// mirserver
#include "screen.h"
#include "screenscontroller.h"
#include "logging.h"

// Qt
#include <QGuiApplication>
#include <QScreen>
#include <qpa/qplatformnativeinterface.h>
#include <QDebug>

using namespace qtmir;

#define DEBUG_MSG qCDebug(QTMIR_SCREENS).nospace() << "QQuickScreenWindow[" << (void*)this <<"]::" << __func__

QQuickScreenWindow::QQuickScreenWindow(QQuickWindow *parent)
    : QQuickWindow(parent)
{
    if (qGuiApp->platformName() != QLatin1String("mirserver")) {
        qCritical("Not using 'mirserver' QPA plugin. Using ScreenWindow may produce unknown results.");
    }

    DEBUG_MSG << "()";
}

QQuickScreenWindow::~QQuickScreenWindow()
{
    DEBUG_MSG << "()";
}

ScreenAdapter *QQuickScreenWindow::screenWrapper() const
{
    return m_screen.data();
}

void QQuickScreenWindow::setScreenWrapper(ScreenAdapter *screen)
{
    DEBUG_MSG << "(screen=" << screen << ")";
    if (m_screen != screen) {
        m_screen = screen;
        Q_EMIT screenWrapperChanged();
    }
    QQuickWindow::setScreen(screen->screen());
}
