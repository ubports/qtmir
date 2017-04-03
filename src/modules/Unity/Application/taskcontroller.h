/*
 * Copyright (C) 2014-2017 Canonical, Ltd.
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
 *
 */

#ifndef QTMIR_TASK_CONTROLLER_H
#define QTMIR_TASK_CONTROLLER_H

#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

// miral
#include <miral/application.h>
#include <miral/application_info.h>

// mirserver
#include "promptsessionmanager.h"

namespace mir {
    namespace scene {
        class Session;
        class PromptSession;
    }
}

namespace unity {
    namespace shell {
        namespace application {
            class MirSurfaceInterface;
        }
    }
}

class PromptSessionListener;

namespace qtmir
{

class AppNotifier;
class ApplicationInfo;
class SessionInterface;

class TaskController : public QObject
{
    Q_OBJECT

public:
    enum class Error
    {
        APPLICATION_CRASHED,
        APPLICATION_FAILED_TO_START
    };

    virtual ~TaskController() = default;

    TaskController& operator=(const TaskController&) = delete;

    virtual bool appIdHasProcessId(const QString &appId, pid_t pid) = 0;

    virtual bool stop(const QString &appId) = 0;
    virtual bool start(const QString &appId, const QStringList &arguments) = 0;

    virtual bool suspend(const QString &appId) = 0;
    virtual bool resume(const QString &appId) = 0;

    virtual QSharedPointer<qtmir::ApplicationInfo> getInfoForApp(const QString &appId) const = 0;

    SessionInterface *findSession(const mir::scene::Session* session) const;

Q_SIGNALS:
    void processStarting(const QString &appId);
    void applicationStarted(const QString &appId);
    void processStopped(const QString &appId);
    void processSuspended(const QString &appId);
    void focusRequested(const QString &appId);
    void resumeRequested(const QString &appId);

    void processFailed(const QString &appId, TaskController::Error error);

    void authorizationRequestedForSession(const pid_t &pid, bool &authorized);

    void sessionStarting(SessionInterface *session);

public Q_SLOTS:
    void onSessionStarting(const miral::ApplicationInfo &appInfo);
    void onSessionStopping(const miral::ApplicationInfo &appInfo);

    void onPromptSessionStarting(const PromptSession& promptSession);
    void onPromptSessionStopping(const PromptSession& promptSession);
    void onPromptProviderAdded(const qtmir::PromptSession &promptSession, const miral::Application &);
    void onPromptProviderRemoved(const qtmir::PromptSession &promptSession, const miral::Application &);

protected:
    TaskController(QObject *parent = nullptr);
    TaskController(std::shared_ptr<PromptSessionManager>&, QObject *parent = nullptr);

private:
    void connectToAppNotifier(AppNotifier *);
    void connectToPromptSessionListener(PromptSessionListener *);

    std::shared_ptr<PromptSessionManager> m_promptSessionManager;

    QHash<const mir::scene::PromptSession *, SessionInterface *> m_mirPromptToSessionHash;
    QList<SessionInterface*> m_sessionList;
};

} // namespace qtmir

#endif // QTMIR_TASK_CONTROLLER_H
