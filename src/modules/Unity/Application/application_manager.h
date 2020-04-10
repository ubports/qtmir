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

#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

// std
#include <memory>

// Qt
#include <QObject>
#include <QStringList>

// local
#include "application.h"
#include "sessionmap_interface.h"
#include "taskcontroller.h"

// Unity API
#include <unity/shell/application/ApplicationManagerInterface.h>

namespace mir {
    namespace scene {
        class Session;
        class Surface;
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

namespace qtmir {

class DBusFocusInfo;
class DBusWindowStack;
class ProcInfo;
class SharedWakelock;
class SettingsInterface;

class ApplicationManager : public unity::shell::application::ApplicationManagerInterface,
                           public SessionMapInterface
{
    Q_OBJECT

public:
    static ApplicationManager* create();
    static ApplicationManager* singleton();

    // Noone else will use the objects passed in this contructor
    explicit ApplicationManager(
            const QSharedPointer<TaskController> &taskController,
            const QSharedPointer<SharedWakelock> &sharedWakelock,
            const QSharedPointer<ProcInfo> &processInfo,
            const QSharedPointer<SettingsInterface> &settings,
            QObject *parent = 0);
    virtual ~ApplicationManager();

    // unity::shell::application::ApplicationManagerInterface
    QString focusedApplicationId() const override;
    Q_INVOKABLE qtmir::Application* get(int index) const override;
    Q_INVOKABLE qtmir::Application* findApplication(const QString &appId) const override;
    unity::shell::application::ApplicationInfoInterface *findApplicationWithSurface(unity::shell::application::MirSurfaceInterface* surface) const override;
    Q_INVOKABLE bool requestFocusApplication(const QString &appId) override;
    Q_INVOKABLE qtmir::Application* startApplication(const QString &appId, const QStringList &arguments = QStringList()) override;
    Q_INVOKABLE bool stopApplication(const QString &appId) override;

    // QAbstractListModel
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role) const override;

    SessionInterface *findSession(const mir::scene::Session* session) const override;

public Q_SLOTS:
    void authorizeSession(const pid_t pid, bool &authorized);

    void onProcessStarting(const QString& appId);
    void onProcessStopped(const QString& appId);
    void onProcessSuspended(const QString& appId);
    void onProcessFailed(const QString& appId, TaskController::Error error);
    void onFocusRequested(const QString& appId);
    void onResumeRequested(const QString& appId);
    void onSessionStarting(SessionInterface *session);

private Q_SLOTS:
    void onAppDataChanged(const int role);
    void onApplicationClosing(Application *application);
    void addApp(const QSharedPointer<qtmir::ApplicationInfo> &appInfo, const QStringList &arguments, const pid_t pid);

Q_SIGNALS:
    void queuedAddApp(const QSharedPointer<qtmir::ApplicationInfo> &appInfo, const QStringList &arguments, const pid_t pid);

private:
    // All calls to private functions happen with the mutex held
    Application* findApplicationMutexHeld(const QString &inputAppId) const;

    Application* findApplicationWithSession(const std::shared_ptr<mir::scene::Session> &session) const;
    void setFocused(Application *application);
    void add(Application *application);
    void remove(Application* application);

    QModelIndex findIndex(Application* application);
    void resumeApplication(Application *application);
    QString toString() const;

    Application* findApplicationWithPromptSession(const mir::scene::PromptSession* promptSession);
    Application *findClosingApplication(const QString &inputAppId) const;
    QSharedPointer<qtmir::ApplicationInfo> tryFindApp(const pid_t pid);

    QList<Application*> m_applications;
    DBusFocusInfo *m_dbusFocusInfo;
    QSharedPointer<TaskController> m_taskController;
    QSharedPointer<ProcInfo> m_procInfo;
    QSharedPointer<SharedWakelock> m_sharedWakelock;
    QSharedPointer<SettingsInterface> m_settings;
    QList<Application*> m_closingApplications;
    QList<QString> m_queuedStartApplications;
    bool m_modelUnderChange{false};
    static ApplicationManager* the_application_manager;

    QHash<pid_t, QString> m_authorizedPids;

    mutable QMutex m_mutex;
};

} // namespace qtmir

Q_DECLARE_METATYPE(qtmir::ApplicationManager*)

#endif // APPLICATIONMANAGER_H
