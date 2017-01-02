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

#ifndef POINTERPOSITION_H
#define POINTERPOSITION_H

#include <QObject>

class PointerPosition : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int x READ x NOTIFY xChanged)
    Q_PROPERTY(int y READ y NOTIFY yChanged)

public:
    static PointerPosition *instance();

    int x() const { return m_x; }
    int y() const { return m_y; }

Q_SIGNALS:
    void xChanged();
    void yChanged();

protected:
    bool eventFilter(QObject *object, QEvent *event);

private:
    Q_DISABLE_COPY(PointerPosition)
    PointerPosition();
    ~PointerPosition();

    int m_x{0}, m_y{0};
};

#endif // POINTERPOSITION_H
