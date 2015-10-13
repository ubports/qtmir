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

#ifndef QTMIR_CURSOR_H
#define QTMIR_CURSOR_H

#include <QMutex>
#include <QPointer>

// Unity API
#include <unity/shell/application/MirPlatformCursor.h>

namespace qtmir {

class Cursor : public MirPlatformCursor
{
public:
    Cursor();

    // Called form Mir input thread
    bool handleMouseEvent(ulong timestamp, QPointF movement, Qt::MouseButton buttons,
            Qt::KeyboardModifiers modifiers);

    ////
    // MirPlatformCursor

    // Called from Qt's GUI thread
    void setMousePointer(MirMousePointerInterface *mousePointer) override;

    ////
    // QPlatformCursor

    void changeCursor(QCursor *windowCursor, QWindow *window) override;

    void setPos(const QPoint &pos) override;
    QPoint pos() const override;

private Q_SLOTS:
    void setMirCursorName(const QString &mirCursorName);

private:
    void updateMousePointerCursorName();
    QMutex m_mutex;
    QPointer<MirMousePointerInterface> m_mousePointer;
    QMap<int,QString> m_shapeToCursorName;
    QString m_qtCursorName;
    QString m_mirCursorName;
};

} // namespace qtmir

#endif // QTMIR_CURSOR_H
