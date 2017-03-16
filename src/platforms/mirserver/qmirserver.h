/*
 * Copyright (C) 2013-2017 Canonical, Ltd.
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
#include <QSharedPointer>

#include <memory>

// local
#include "qtmir/sessionauthorizer.h"
#include "qtmir/windowmanagementpolicy.h"
#include "qtmir/displayconfigurationpolicy.h"
#include "qtmir/displayconfigurationstorage.h"

// qtmir
namespace qtmir {
    class PromptSessionManager;
    class WindowModelNotifier;
    class AppNotifier;
}
namespace mir { class Server; }

class QMirServerPrivate;
class ScreensController;
class ScreensModel;
class QPlatformOpenGLContext;
class QOpenGLContext;

class QMirServer: public QObject
{
    Q_OBJECT

public:
    virtual ~QMirServer();

    static QSharedPointer<QMirServer> create();

    void start();
    Q_SLOT void stop();
    bool isRunning() const;

    QSharedPointer<ScreensModel> screensModel() const;
    QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const;
    void *nativeResourceForIntegration(const QByteArray &resource) const;
    std::shared_ptr<qtmir::PromptSessionManager> thePromptSessionManager() const;

    qtmir::WindowModelNotifier *windowModelNotifier() const;
    qtmir::AppNotifier *appNotifier() const;

    void wrapDisplayConfigurationPolicy(qtmir::DisplayConfigurationPolicyWrapper const& setDisplayConfigurationPolicy);
    void overrideSessionAuthorizer(qtmir::SessionAuthorizerBuilder const& setApplicationAuthorizer);
    void overrideWindowManagementPolicy(qtmir::WindowManagmentPolicyBuilder const& wmPolicyCreator);
    void overrideDisplayConfigurationStorage(qtmir::BasicSetDisplayConfigurationStorage const& setDisplayConfigStorage);

Q_SIGNALS:
    void started();
    void stopped();

protected:
    QMirServerPrivate * const d_ptr;

private:
    QMirServer(QObject *parent = nullptr);
    Q_DISABLE_COPY(QMirServer)
    Q_DECLARE_PRIVATE(QMirServer)
};

#endif // QMIRSERVER_H
