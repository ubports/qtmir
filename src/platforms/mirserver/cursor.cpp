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
    m_shapeToCursorName[Qt::ArrowCursor] = QStringLiteral("left_ptr");
    m_shapeToCursorName[Qt::UpArrowCursor] = QStringLiteral("up_arrow");
    m_shapeToCursorName[Qt::CrossCursor] = QStringLiteral("cross");
    m_shapeToCursorName[Qt::WaitCursor] = QStringLiteral("watch");
    m_shapeToCursorName[Qt::IBeamCursor] = QStringLiteral("xterm");
    m_shapeToCursorName[Qt::SizeVerCursor] = QStringLiteral("size_ver");
    m_shapeToCursorName[Qt::SizeHorCursor] = QStringLiteral("size_hor");
    m_shapeToCursorName[Qt::SizeBDiagCursor] = QStringLiteral("size_bdiag");
    m_shapeToCursorName[Qt::SizeFDiagCursor] = QStringLiteral("size_fdiag");
    m_shapeToCursorName[Qt::SizeAllCursor] = QStringLiteral("size_all");
    m_shapeToCursorName[Qt::BlankCursor] = QStringLiteral("blank");
    m_shapeToCursorName[Qt::SplitVCursor] = QStringLiteral("split_v");
    m_shapeToCursorName[Qt::SplitHCursor] = QStringLiteral("split_h");
    m_shapeToCursorName[Qt::PointingHandCursor] = QStringLiteral("pointing_hand");
    m_shapeToCursorName[Qt::ForbiddenCursor] = QStringLiteral("forbidden");
    m_shapeToCursorName[Qt::WhatsThisCursor] = QStringLiteral("whats_this");
    m_shapeToCursorName[Qt::BusyCursor] = QStringLiteral("left_ptr_watch");
    m_shapeToCursorName[Qt::OpenHandCursor] = QStringLiteral("openhand");
    m_shapeToCursorName[Qt::ClosedHandCursor] = QStringLiteral("closedhand");
    m_shapeToCursorName[Qt::DragCopyCursor] = QStringLiteral("copy");
    m_shapeToCursorName[Qt::DragMoveCursor] = QStringLiteral("move");
    m_shapeToCursorName[Qt::DragLinkCursor] = QStringLiteral("link");

    connect(Mir::instance(), &Mir::cursorNameChanged, this, &Cursor::setMirCursorName);
}

void Cursor::changeCursor(QCursor *windowCursor, QWindow * /*window*/)
{
    if (m_mousePointer.isNull()) {
        return;
    }

    if (windowCursor) {
        m_qtCursorName = m_shapeToCursorName.value(windowCursor->shape(), QStringLiteral("left_ptr"));
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
        Q_ARG(Qt::MouseButtons, buttons),
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
            m_mousePointer->setCursorName(QStringLiteral("left_ptr"));
        } else {
            m_mousePointer->setCursorName(m_qtCursorName);
        }
    } else {
        m_mousePointer->setCursorName(m_mirCursorName);
    }
}
