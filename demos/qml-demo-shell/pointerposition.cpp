/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pointerposition.h"

#include <QGuiApplication>
#include <QMouseEvent>

PointerPosition *PointerPosition::instance()
{
    static PointerPosition *pointerPosition = nullptr;
    if (!pointerPosition) {
        pointerPosition = new PointerPosition();
    }
    return pointerPosition;
}

bool PointerPosition::eventFilter(QObject */*object*/, QEvent *event)
{
    if (event->type() == QEvent::MouseMove) {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        if (m_x != mouseEvent->globalX()) {
            m_x = mouseEvent->globalX();
            Q_EMIT xChanged();
        }
        if (m_y != mouseEvent->globalY()) {
            m_y = mouseEvent->globalY();
            Q_EMIT yChanged();
        }
    }
    return false;
}

PointerPosition::PointerPosition()
    : QObject()
{
    qGuiApp->installEventFilter(this);
}

PointerPosition::~PointerPosition()
{
    qGuiApp->removeEventFilter(this);
}
