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

// mirserver
#include "screen.h"
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
    auto app = static_cast<QGuiApplication *>(QGuiApplication::instance());
    if (!app) {
        return;
    }
    connect(app, &QGuiApplication::screenAdded, this, &Screens::onScreenAdded);
    connect(app, &QGuiApplication::screenRemoved, this, &Screens::onScreenRemoved);

    m_screenList = QGuiApplication::screens();
    DEBUG_MSG << "(" << m_screenList << ")";
}

QHash<int, QByteArray> Screens::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ScreenRole] = "screen";
    roles[OutputTypeRole] = "outputType";
    roles[EnabledRole] = "enabled";
    roles[NameRole] = "name";
    roles[ScaleRole] = "scale";
    roles[FormFactorRole] = "formFactor";
    roles[GeometryRole] = "geometry";
    roles[SizesRole] = "sizes";
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
    case OutputTypeRole: {
        auto screen = static_cast<Screen*>(m_screenList.at(index.row())->handle());
        if (screen) {
            return QVariant(screen->outputType());
        } else {
            return OutputTypes::Unknown;
        }
    }
    case EnabledRole: {
        auto screen = static_cast<Screen*>(m_screenList.at(index.row())->handle());
        if (screen) {
            return screen->used();
        } else {
            return false;
        }
    }
    case NameRole: {
        auto screen = static_cast<Screen*>(m_screenList.at(index.row())->handle());
        if (screen) {
            return screen->name();
        } else {
            return QString();
        }
    }
    case ScaleRole: {
        auto screen = static_cast<Screen*>(m_screenList.at(index.row())->handle());
        if (screen) {
            return QVariant(screen->scale());
        } else {
            return 1.0;
        }
    }
    case FormFactorRole: {
        auto screen = static_cast<Screen*>(m_screenList.at(index.row())->handle());
        if (screen) {
            return QVariant(static_cast<FormFactor>(screen->formFactor())); //FIXME: cheeky
        } else {
            return FormFactor::FormFactorUnknown;
        }
    }
    case GeometryRole: {
        auto screen = static_cast<Screen*>(m_screenList.at(index.row())->handle());
        if (screen) {
            return screen->geometry();
        } else {
            return QRect();
        }
    }
    case SizesRole: {
        auto screen = static_cast<Screen*>(m_screenList.at(index.row())->handle());
        if (screen) {
            QVariantList sizes;
            auto availableSizes = screen->availableSizes();
            Q_FOREACH(auto size, availableSizes) {
                sizes.append(QVariant(size));
            }
            return sizes;
        } else {
            return QVariantList();
        }
    }
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

void Screens::activateScreen(int index)
{
    if (index < 0 || m_screenList.count() <= index) return;

    auto platformScreen = static_cast<Screen*>(m_screenList.at(index)->handle());
    if (platformScreen && platformScreen->primaryWindow()) {
        auto window = platformScreen->primaryWindow()->window();
        if (window) {
            window->requestActivate();
        }
    }
}

void Screens::onScreenAdded(QScreen *screen)
{
    if (m_screenList.contains(screen))
        return;
    DEBUG_MSG << "(screen=" << screen << ")";

    beginInsertRows(QModelIndex(), count(), count());
    m_screenList.push_back(screen);
    endInsertRows();
    Q_EMIT screenAdded(screen);
    Q_EMIT countChanged();
}

void Screens::onScreenRemoved(QScreen *screen)
{
    int index = m_screenList.indexOf(screen);
    if (index < 0)
        return;
    DEBUG_MSG << "(screen=" << screen << ")";

    beginRemoveRows(QModelIndex(), index, index);
    m_screenList.removeAt(index);
    endRemoveRows();
    Q_EMIT screenRemoved(screen);
    Q_EMIT countChanged();
}

} // namespace qtmir
