/*
 * Copyright © 2016 Canonical Ltd.
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

#ifndef UNITY_SCREEN_TYPES_H
#define UNITY_SCREEN_TYPES_H

#include <QObject>
#include <QSize>

class ScreenMode : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal refreshRate MEMBER refreshRate CONSTANT)
    Q_PROPERTY(QSize size MEMBER size CONSTANT)
public:
    ScreenMode():refreshRate(-1) {}
    ScreenMode(const ScreenMode& other)
        : QObject(nullptr),
          refreshRate{other.refreshRate},size{other.size}
    {}

    qreal refreshRate;
    QSize size;
};

Q_DECLARE_METATYPE(ScreenMode)

#endif //UNITY_SCREEN_TYPES_H
