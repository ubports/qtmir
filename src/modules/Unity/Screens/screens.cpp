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
 */

#include "screens.h"

#include <QGuiApplication>
#include <QScreen>

Q_DECLARE_METATYPE(QScreen*)

namespace qtmir {

Screens::Screens(QObject *parent) :
    QAbstractListModel(parent)
{
    auto app = static_cast<QGuiApplication *>(QGuiApplication::instance());
    if (!app) {
        return;
    }
    connect(app, &QGuiApplication::screenAdded, this, &Screens::onScreenAdded);
    connect(app, &QGuiApplication::screenRemoved, this, &Screens::onScreenRemoved);
}

QVariant Screens::data(const QModelIndex &index, int) const
{
    QList<QScreen *> qscreenList = QGuiApplication::screens();

    if (!index.isValid() || index.row() >= qscreenList.size()) {
        return QVariant();
    }

    return QVariant::fromValue(qscreenList.at(index.row()));
}

int Screens::rowCount(const QModelIndex &) const
{
    return count();
}

int Screens::count() const
{
    return QGuiApplication::screens().size();
}

void Screens::onScreenAdded(QScreen *screen)
{
    Q_EMIT screenAdded(screen);
    Q_EMIT countChanged();
}

void Screens::onScreenRemoved(QScreen *screen)
{
    Q_EMIT screenRemoved(screen);
    Q_EMIT countChanged();
}


} // namespace qtmir
