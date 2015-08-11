/*
 * Copyright (C) 2011-2015 Canonical, Ltd.
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

#ifndef ABSTRACTDBUSSERVICEMONITOR_H
#define ABSTRACTDBUSSERVICEMONITOR_H

#include <QObject>
#include <QString>
#include <QDBusConnection>

class QDBusAbstractInterface;
class QDBusServiceWatcher;

class Q_DECL_EXPORT AbstractDBusServiceMonitor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool serviceAvailable READ serviceAvailable NOTIFY serviceAvailableChanged)

public:
    explicit AbstractDBusServiceMonitor(const QString &service, const QString &path, const QString &interface,
                                        const QDBusConnection &connection = QDBusConnection::sessionBus(),
                                        QObject *parent = 0);
    ~AbstractDBusServiceMonitor();

    QDBusAbstractInterface* dbusInterface() const;

    bool serviceAvailable() const;

Q_SIGNALS:
    void serviceAvailableChanged(bool available);

private Q_SLOTS:
    void createInterface(const QString &service);
    void destroyInterface(const QString &service);

protected:
    const QString m_service;
    const QString m_path;
    const QString m_interface;
    const QDBusConnection m_busConnection;
    QDBusServiceWatcher* m_watcher;
    QDBusAbstractInterface* m_dbusInterface;
};

#endif // ABSTRACTDBUSSERVICEMONITOR_H
