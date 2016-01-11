/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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

class QMirServerPrivate;
class MirServer;
class ScreensController;
class ScreensModel;

class QMirServer: public QObject
{
    Q_OBJECT

public:
    QMirServer(const QStringList &arguments, QObject* parent=0);
    virtual ~QMirServer();

    bool start();
    Q_SLOT void stop();
    bool isRunning() const;

    QWeakPointer<MirServer> mirServer() const;

    QWeakPointer<ScreensController> screensController() const;
    QWeakPointer<ScreensModel> screensModel() const;

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
