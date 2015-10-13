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
    m_shapeToCursorName[Qt::PointingHandCursor] = "pointing_hand";
    m_shapeToCursorName[Qt::ForbiddenCursor] = "forbidden";
    m_shapeToCursorName[Qt::WhatsThisCursor] = "whats_this";
    m_shapeToCursorName[Qt::BusyCursor] = "left_ptr_watch";
    m_shapeToCursorName[Qt::OpenHandCursor] = "openhand";
    m_shapeToCursorName[Qt::ClosedHandCursor] = "closedhand";
    m_shapeToCursorName[Qt::DragCopyCursor] = "copy";
    m_shapeToCursorName[Qt::DragMoveCursor] = "move";
    m_shapeToCursorName[Qt::DragLinkCursor] = "link";

    connect(Mir::instance(), &Mir::cursorNameChanged, this, &Cursor::setMirCursorName);
}

void Cursor::changeCursor(QCursor *windowCursor, QWindow * /*window*/)
{
    if (m_mousePointer.isNull()) {
        return;
    }

    if (windowCursor) {
        m_qtCursorName = m_shapeToCursorName.value(windowCursor->shape(), QString("left_ptr"));
    } else {
        m_qtCursorName.clear();
    }

    updateMousePointerCursorName();
}

void Cursor::setMirCursorName(const QString &mirCursorName)
{
    m_mirCursorName = mirCursorName;
    updateMousePointerCursorName();
}

void Cursor::setMousePointer(MirMousePointerInterface *mousePointer)
{
    QMutexLocker locker(&m_mutex);

    if (mousePointer && !m_mousePointer.isNull()) {
        qFatal("QPA mirserver: Only one MousePointer per screen is allowed!");
    }

    m_mousePointer = mousePointer;
    updateMousePointerCursorName();
}

bool Cursor::handleMouseEvent(ulong timestamp, QPointF movement, Qt::MouseButtons buttons,
        Qt::KeyboardModifiers modifiers)
{
    QMutexLocker locker(&m_mutex);

    if (!m_mousePointer || !m_mousePointer->isVisible()) {
        return false;
    }

    // Must not be called directly as we're most likely not in Qt's GUI (main) thread.
    bool ok = QMetaObject::invokeMethod(m_mousePointer, "handleMouseEvent", Qt::AutoConnection,
        Q_ARG(ulong, timestamp),
        Q_ARG(QPointF, movement),
        Q_ARG(Qt::MouseButton, static_cast<Qt::MouseButton>((int)buttons)), // FIXME in MirMousePointerInterface!!!
        Q_ARG(Qt::KeyboardModifiers, modifiers));

    if (!ok) {
        qCWarning(QTMIR_MIR_INPUT) << "Failed to invoke MousePointer::handleMouseEvent";
    }

    return ok;
}

void Cursor::setPos(const QPoint &pos)
{
    if (!m_mousePointer) {
        QPlatformCursor::setPos(pos);
        return;
    }

    QPointF movement;
    QPointF mouseScenePos = m_mousePointer->mapToItem(nullptr, QPointF(0, 0));

    movement.setX(pos.x() - mouseScenePos.x());
    movement.setY(pos.y() - mouseScenePos.y());

    m_mousePointer->handleMouseEvent(0 /*timestamp*/, movement, Qt::NoButton, Qt::NoModifier);
}

QPoint Cursor::pos() const
{
    if (m_mousePointer) {
        return m_mousePointer->mapToItem(nullptr, QPointF(0, 0)).toPoint();
    } else {
        return QPlatformCursor::pos();
    }
}

void Cursor::updateMousePointerCursorName()
{
    if (!m_mousePointer) {
        return;
    }

    if (m_mirCursorName.isEmpty()) {
        if (m_qtCursorName.isEmpty()) {
            m_mousePointer->setCursorName("left_ptr");
        } else {
            m_mousePointer->setCursorName(m_qtCursorName);
        }
    } else {
        m_mousePointer->setCursorName(m_mirCursorName);
    }
}
