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

    ////
    // MirPlatformCursor

    // Called from Qt's GUI thread
    void registerMousePointer(MirMousePointerInterface *mousePointer) override;
    void unregisterMousePointer(MirMousePointerInterface *mousePointer) override;

    ////
    // QPlatformCursor

    void pointerEvent(const QMouseEvent & event) override;
    void changeCursor(QCursor *windowCursor, QWindow *window) override;

    void setPos(const QPoint &pos) override;
    QPoint pos() const override;

private Q_SLOTS:
    void setMirCursorName(const QString &mirCursorName);

private:
    MirMousePointerInterface *primaryMousePointer() const;

    void updateMousePointerCursor();
    QMutex m_mutex;
    QVector<QPointer<MirMousePointerInterface>> m_mousePointers;
    QMap<int,QString> m_shapeToCursorName;
    QString m_qtCursorName;
    QString m_mirCursorName;
    QCursor* m_customCursor;
};

} // namespace qtmir

#endif // QTMIR_CURSOR_H
