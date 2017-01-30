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
    DEBUG_MSG << "()";
}

QQuickScreenWindow::~QQuickScreenWindow()
{
    DEBUG_MSG << "()";
}

Screen *QQuickScreenWindow::screen() const
{
    auto screen = QQuickWindow::screen();
    if (!screen) return nullptr;

    return static_cast<Screen*>(screen->handle());
}

void QQuickScreenWindow::setScreen(Screen *screen)
{
    DEBUG_MSG << "(screen=" << screen << ")";
    QQuickWindow::setScreen(screen->screen());
}
