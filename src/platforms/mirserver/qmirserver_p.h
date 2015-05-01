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

#ifndef QMIRSERVER_P_H
#define QMIRSERVER_P_H

// Qt
#include <QThread>
#include <QSharedPointer>

// std
#include <condition_variable>
#include <mutex>

// local
#include "mirserver.h"

class QMirServer;
class MirServerWorker;

struct QMirServerPrivate
{
    QSharedPointer<MirServer> server;
    QThread serverThread;
    MirServerWorker *serverWorker;
};


// Wraps mir::Server with QObject, so it can be controlled via QThread
class MirServerWorker : public QObject
{
    Q_OBJECT

public:
    MirServerWorker(const QSharedPointer<MirServer> &server)
        : server(server)
    {}

    bool waitForMirStartup();

Q_SIGNALS:
    void stopped();

public Q_SLOTS:
    void run();
    void stop();

private:
    std::mutex mutex;
    std::condition_variable started_cv;
    bool mir_running{false};

    const QSharedPointer<MirServer> server;
};

#endif // QMIRSERVER_P_H
