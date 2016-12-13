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

#ifndef QMIRSERVER_H
#define QMIRSERVER_H

// Qt
#include <QObject>
#include <QWeakPointer>

#include <memory>

// qtmir
namespace qtmir { class PromptSessionManager; }

class QMirServerPrivate;
class ScreensController;
class ScreensModel;
class QPlatformOpenGLContext;
class QOpenGLContext;

class QMirServer: public QObject
{
    Q_OBJECT

public:
    QMirServer(int &argc, char **argv, QObject* parent=0);
    virtual ~QMirServer();

    void start();
    Q_SLOT void stop();
    bool isRunning() const;

    QSharedPointer<ScreensModel> screensModel() const;
    QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const;
    void *nativeResourceForIntegration(const QByteArray &resource) const;
    std::shared_ptr<qtmir::PromptSessionManager> thePromptSessionManager() const;

Q_SIGNALS:
    void started();
    void stopped();

protected:
    QMirServerPrivate * const d_ptr;

private:
    Q_DISABLE_COPY(QMirServer)
    Q_DECLARE_PRIVATE(QMirServer)
};

#endif // QMIRSERVER_H
