/*
 * Copyright (C) 2017 Canonical, Ltd.
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

#ifndef QTMIR_SCREENS_H
#define QTMIR_SCREENS_H

#include <QObject>
#include <QVector>

class QScreen;

namespace qtmir
{
class Screen;

class Screens : public QObject
{
    Q_OBJECT
public:
    Screens(QObject *parent = 0): QObject(parent) {}
    ~Screens() = default;

    virtual QVector<qtmir::Screen*> screens() const = 0;

Q_SIGNALS:
    void screenAdded(qtmir::Screen *screen);
    void screenRemoved(qtmir::Screen *screen);
};

} // namespace qtmir

#endif // QTMIR_SCREENS_H
