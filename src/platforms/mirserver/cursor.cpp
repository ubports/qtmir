/*
 * Copyright (C) 2015 Canonical, Ltd.
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
 *
 */

#include "cursor.h"
#include "logging.h"

#include "mirsingleton.h"

// Unity API
#include <unity/shell/application/MirMousePointerInterface.h>

using namespace qtmir;

Cursor::Cursor()
    : m_customCursor(nullptr)
{
    m_shapeToCursorName[Qt::ArrowCursor] = "left_ptr";
    m_shapeToCursorName[Qt::UpArrowCursor] = "up_arrow";
    m_shapeToCursorName[Qt::CrossCursor] = "cross";
    m_shapeToCursorName[Qt::WaitCursor] = "watch";
    m_shapeToCursorName[Qt::IBeamCursor] = "xterm";
    m_shapeToCursorName[Qt::SizeVerCursor] = "size_ver";
    m_shapeToCursorName[Qt::SizeHorCursor] = "size_hor";
    m_shapeToCursorName[Qt::SizeBDiagCursor] = "size_bdiag";
    m_shapeToCursorName[Qt::SizeFDiagCursor] = "size_fdiag";
    m_shapeToCursorName[Qt::SizeAllCursor] = "size_all";
    m_shapeToCursorName[Qt::BlankCursor] = "blank";
    m_shapeToCursorName[Qt::SplitVCursor] = "split_v";
    m_shapeToCursorName[Qt::SplitHCursor] = "split_h";
    m_shapeToCursorName[Qt::PointingHandCursor] = "hand";
    m_shapeToCursorName[Qt::ForbiddenCursor] = "forbidden";
    m_shapeToCursorName[Qt::WhatsThisCursor] = "whats_this";
    m_shapeToCursorName[Qt::BusyCursor] = "left_ptr_watch";
    m_shapeToCursorName[Qt::OpenHandCursor] = "openhand";
    m_shapeToCursorName[Qt::ClosedHandCursor] = "closedhand";
    m_shapeToCursorName[Qt::DragCopyCursor] = "dnd-copy";
    m_shapeToCursorName[Qt::DragMoveCursor] = "dnd-move";
    m_shapeToCursorName[Qt::DragLinkCursor] = "dnd-link";

    connect(Mir::instance(), &Mir::cursorNameChanged, this, &Cursor::setMirCursorName);
}

void Cursor::changeCursor(QCursor *windowCursor, QWindow * /*window*/)
{
    delete m_customCursor;
    m_customCursor = windowCursor ? new QCursor(*windowCursor) : nullptr;

    updateMousePointerCursor();
}

void Cursor::setMirCursorName(const QString &mirCursorName)
{
    m_mirCursorName = mirCursorName;
    updateMousePointerCursor();
}

MirMousePointerInterface *Cursor::primaryMousePointer() const
{
    return m_mousePointers.value(0).data();
}

void Cursor::registerMousePointer(MirMousePointerInterface *mousePointer)
{
    QMutexLocker locker(&m_mutex);
    if (!mousePointer || m_mousePointers.contains(mousePointer)) return;
    m_mousePointers.push_back(mousePointer);

    updateMousePointerCursor();
}

void Cursor::unregisterMousePointer(MirMousePointerInterface *mousePointer)
{
    QMutexLocker locker(&m_mutex);

    int index = m_mousePointers.indexOf(mousePointer);
    if (index > 0) {
        m_mousePointers.remove(index);
        if (index == 0) {
            updateMousePointerCursor();
        }
    }
}

void Cursor::pointerEvent(const QMouseEvent &event)
{
    QMutexLocker locker(&m_mutex);

    auto pos = event.pos();
    qCDebug(QTMIR_MIR_INPUT) << "Cursor::pointerEvent(x=" << pos.x() << ", y=" << pos.y() << ")";

    Q_FOREACH(auto pointer, m_mousePointers) {
        if (!pointer) continue;
        pointer->setPosition(pos);
    }
}

void Cursor::setPos(const QPoint &pos)
{
    QMutexLocker locker(&m_mutex);
    qCDebug(QTMIR_MIR_INPUT) << "Cursor::setPos(x=" << pos.x() << ", y=" << pos.y() << ")";

    Q_FOREACH(auto pointer, m_mousePointers) {
        if (!pointer) continue;

        pointer->setPosition(pos);
    }
}

QPoint Cursor::pos() const
{
    auto mousePointer = primaryMousePointer();
    if (mousePointer) {
        return mousePointer->mapToItem(nullptr, QPointF(0, 0)).toPoint();
    } else {
        return QPlatformCursor::pos();
    }
}

void Cursor::updateMousePointerCursor()
{
    auto mousePointer = primaryMousePointer();
    if (!mousePointer) return;

    QString name;
    if (m_customCursor) {
        if (m_customCursor->pixmap().isNull()) {
            name = m_shapeToCursorName.value(m_customCursor->shape(), QLatin1String("left_ptr"));
            mousePointer->setCustomCursor(QCursor());
        } else {
            name = QLatin1String("custom");
            mousePointer->setCustomCursor(*m_customCursor);
        }
    } else {
        mousePointer->setCustomCursor(QCursor());
    }

    mousePointer->setCursorName(m_mirCursorName.isEmpty() ? name.isEmpty() ? "left_ptr" : name : m_mirCursorName);
}
