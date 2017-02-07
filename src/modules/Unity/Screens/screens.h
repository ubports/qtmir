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

#ifndef SCREENS_H
#define SCREENS_H

#include "screentypes.h"

#include <QAbstractListModel>

class QScreen;

namespace qtmir {

class Screens : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum ItemRoles {
        ScreenRole = Qt::UserRole + 1
    };

    explicit Screens(QObject *parent = 0);
    virtual ~Screens() noexcept = default;

    /* QAbstractItemModel */
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = ScreenRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int count() const;

Q_SIGNALS:
    void countChanged();
    void screenAdded(QScreen *screen);
    void screenRemoved(QScreen *screen);

private Q_SLOTS:
    void onScreenAdded(QScreen *screen);
    void onScreenRemoved(QScreen *screen);

private:
    QList<QScreen *> m_screenList;
};

} // namespace qtmir

#endif // SCREENS_H
