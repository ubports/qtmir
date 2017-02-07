/*
 * Copyright (C) 2016 Canonical, Ltd.
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
#include "screen.h"

// mirserver
#include "platformscreen.h"
#include "logging.h"

// Qt
#include <QGuiApplication>
#include <QScreen>

Q_DECLARE_METATYPE(QScreen*)

#define DEBUG_MSG qCDebug(QTMIR_SCREENS).nospace() << "Screens[" << (void*)this <<"]::" << __func__

namespace qtmir {

Screens::Screens(QObject *parent) :
    QAbstractListModel(parent)
{
    if (qGuiApp->platformName() != QLatin1String("mirserver")) {
        qCritical("Not using 'mirserver' QPA plugin. Using Screens may produce unknown results.");
    }

    connect(qGuiApp, &QGuiApplication::screenAdded, this, &Screens::onScreenAdded);
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, &Screens::onScreenRemoved);
    connect(qGuiApp, &QGuiApplication::focusWindowChanged, this, &Screens::activeScreenChanged);

    Q_FOREACH(QScreen* screen, QGuiApplication::screens()) {
        m_screenList.push_back(new ScreenAdapter(screen));
    }
    DEBUG_MSG << "(" << m_screenList << ")";
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
    return count();
}

int Screens::count() const
{
    return m_screenList.size();
}

QVariant Screens::activeScreen() const
{
    for (int i = 0; i < m_screenList.count(); i++) {
        if (m_screenList[i]->isActive()) return i;
    }
    return QVariant();
}

void Screens::activateScreen(const QVariant& vindex)
{
    bool ok = false;
    int index = vindex.toInt(&ok);
    if (!ok || index < 0 || m_screenList.count() <= index) return;

    auto screen = static_cast<ScreenAdapter*>(m_screenList.at(index));
    screen->setActive(true);
}

void Screens::onScreenAdded(QScreen *screen)
{
    Q_FOREACH(auto screenWrapper, m_screenList) {
        if (screenWrapper->screen() == screen) return;
    }
    DEBUG_MSG << "(screen=" << screen << ")";

    beginInsertRows(QModelIndex(), count(), count());
    auto screenWrapper(new ScreenAdapter(screen));
    m_screenList.push_back(screenWrapper);
    endInsertRows();
    Q_EMIT screenAdded(screenWrapper);
    Q_EMIT countChanged();
}

void Screens::onScreenRemoved(QScreen *screen)
{
    DEBUG_MSG << "(screen=" << screen << ")";

    int index = 0;
    QMutableListIterator<ScreenAdapter*> iter(m_screenList);
    while(iter.hasNext()) {
        auto screenWrapper = iter.next();
        if (screenWrapper->screen() == screen) {

            beginRemoveRows(QModelIndex(), index, index);
            auto screenWrapper = m_screenList.takeAt(index);
            endRemoveRows();

            Q_EMIT screenRemoved(screenWrapper);
            Q_EMIT countChanged();

            iter.remove();
            screenWrapper->deleteLater();
            break;
        }
        index++;
    }
}

} // namespace qtmir
