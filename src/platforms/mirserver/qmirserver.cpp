/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
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

// Qt
#include <QObject>
#include <QCoreApplication>
#include <QDebug>

// local
#include "mirserver.h"
#include "qmirserver.h"
#include "qmirserver_p.h"
#include "screen.h"

QMirServer::QMirServer(int &argc, char **argv, QObject *parent)
    : QObject(parent)
    , d_ptr(new QMirServerPrivate())
{
    Q_D(QMirServer);

    d->screensModel = QSharedPointer<ScreensModel>(new ScreensModel());

    d->server = QSharedPointer<MirServer>(new MirServer(argc, argv, d->screensModel));

    d->serverThread = new MirServerThread(d->server);

    connect(d->serverThread, &MirServerThread::stopped, this, &QMirServer::stopped);
}

QMirServer::~QMirServer()
{
    stop();
}

bool QMirServer::start()
{
    Q_D(QMirServer);

    d->serverThread->start(QThread::TimeCriticalPriority);

    if (!d->serverThread->waitForMirStartup())
    {
        qCritical() << "ERROR: QMirServer - Mir failed to start";
        return false;
    }
    d->screensModel->update();
    d->screensController = QSharedPointer<ScreensController>(
                               new ScreensController(d->screensModel, d->server->the_display(),
                                                     d->server->the_display_configuration_controller()));
    Q_EMIT started();
    return true;
}

void QMirServer::stop()
{
    Q_D(QMirServer);

    if (d->serverThread->isRunning()) {
        d->screensController.clear();
        d->serverThread->stop();
        if (!d->serverThread->wait(10000)) {
            // do something to indicate fail during shutdown
            qCritical() << "ERROR: QMirServer - Mir failed to shut down correctly, terminating it";
            d->serverThread->terminate();
        }
    }
}

bool QMirServer::isRunning() const
{
    Q_D(const QMirServer);
    return d->serverThread->isRunning();
}

QWeakPointer<MirServer> QMirServer::mirServer() const
{
    Q_D(const QMirServer);
    return d->server.toWeakRef();
}

QWeakPointer<ScreensModel> QMirServer::screensModel() const
{
    Q_D(const QMirServer);
    return d->screensModel;
}

QWeakPointer<ScreensController> QMirServer::screensController() const
{
    Q_D(const QMirServer);
    return d->screensController;
}
