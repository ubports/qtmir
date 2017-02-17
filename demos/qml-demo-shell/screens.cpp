/*
 * Copyright (C) 2016-2017 Canonical, Ltd.
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

// qtmirserver
#include <qtmir/qtmir.h>
#include <qtmir/screens.h>
#include <qtmir/screen.h>
#include <QGuiApplication>

// Qt
#include <QScreen>
#include <QWindow>

Screens::Screens(QObject *parent)
    : QAbstractListModel(parent)
    , m_wrapped(qtmir::get_screen_model())
{
    if (qGuiApp->platformName() != QLatin1String("mirserver")) {
        qCritical("Not using 'mirserver' QPA plugin. Using qGuiApp may produce unknown results.");
    }

    connect(m_wrapped.data(), &qtmir::Screens::screenAdded, this, &Screens::onScreenAdded);
    connect(m_wrapped.data(), &qtmir::Screens::screenRemoved, this, &Screens::onScreenRemoved);

    Q_FOREACH(qtmir::Screen* screen, m_wrapped->screens()) {
        m_screenList.push_back(screen);
    }
}

QHash<int, QByteArray> Screens::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ScreenRole] = "screen";
    return roles;
}

QVariant Screens::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_screenList.size()) {
        return QVariant();
    }

    switch(role) {
    case ScreenRole:
        return QVariant::fromValue(m_screenList.at(index.row()));
    } // switch

    return QVariant();
}

int Screens::rowCount(const QModelIndex &) const
{
    return m_screenList.size();
}

void Screens::onScreenAdded(qtmir::Screen *screen)
{
    Q_FOREACH(auto screenWrapper, m_screenList) {
        if (screenWrapper == screen) return;
    }

    beginInsertRows(QModelIndex(), m_screenList.count(), m_screenList.count());
    m_screenList.push_back(screen);
    endInsertRows();
    Q_EMIT screenAdded(screen);
}

void Screens::onScreenRemoved(qtmir::Screen *screen)
{
    int index = 0;
    QMutableListIterator<qtmir::Screen*> iter(m_screenList);
    while(iter.hasNext()) {
        auto screenWrapper = iter.next();
        if (screenWrapper == screen) {

            beginRemoveRows(QModelIndex(), index, index);
            iter.remove();
            endRemoveRows();

            Q_EMIT screenRemoved(screen);
            break;
        }
        index++;
    }
}
