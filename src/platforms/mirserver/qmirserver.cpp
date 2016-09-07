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
#include <QOpenGLContext>

// local
#include "mirserver.h"
#include "qmirserver.h"
#include "qmirserver_p.h"
#include "screen.h"
#include "miropenglcontext.h"

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

void QMirServer::start()
{
    Q_D(QMirServer);

    d->serverThread->start(QThread::TimeCriticalPriority);

    if (!d->serverThread->waitForMirStartup()) {
        qFatal("ERROR: QMirServer - Mir failed to start"); // will core dump
    }
    d->screensModel->update();
    d->screensController = QSharedPointer<ScreensController>(
                               new ScreensController(d->screensModel, d->server->the_display(),
                                                     d->server->the_display_configuration_controller()));
    Q_EMIT started();
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

QPlatformOpenGLContext *QMirServer::createPlatformOpenGLContext(QOpenGLContext *context) const
{
    Q_D(const QMirServer);
    return new MirOpenGLContext(*d->server->the_display(), *d->server->the_gl_config(), context->format());
}

void *QMirServer::nativeResourceForIntegration(const QByteArray &resource) const
{
    Q_D(const QMirServer);
    void *result = nullptr;

    if (d->server) {
        if (resource == "SessionAuthorizer")
            result = d->server->sessionAuthorizer();
        else if (resource == "Shell")
            result = d->server->shell();
        else if (resource == "SessionListener")
            result = d->server->sessionListener();
        else if (resource == "PromptSessionListener")
            result = d->server->promptSessionListener();
        else if (resource == "WindowManager")
            result = d->server->windowManager();
        else if (resource == "ScreensController")
            result = screensController().data();
    }
    return result;
}

std::shared_ptr<mir::scene::PromptSessionManager> QMirServer::thePromptSessionManager() const
{
    Q_D(const QMirServer);

    return d->server->the_prompt_session_manager();
}

std::shared_ptr<mir::shell::PersistentSurfaceStore> QMirServer::thePersistentSurfaceStore() const
{
    Q_D(const QMirServer);

    return d->server->the_persistent_surface_store();
}
