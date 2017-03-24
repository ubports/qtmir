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

    // TODO: Move to unity::shell::application::ApplicationManagerInterface
    Q_PROPERTY(bool empty READ isEmpty NOTIFY emptyChanged)

public:
    static ApplicationManager* create();
    static ApplicationManager* singleton();

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

    Q_INVOKABLE void move(int from, int to);

    bool isEmpty() const { return rowCount() == 0; }

    const QList<Application*> &list() const { return m_applications; }
    qtmir::Application* findApplicationWithPid(const pid_t pid) const;

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

Q_SIGNALS:
    void emptyChanged();

private Q_SLOTS:
    void onAppDataChanged(const int role);
    void onApplicationClosing(Application *application);

private:
    Application* findApplicationWithSession(const std::shared_ptr<mir::scene::Session> &session);
    void setFocused(Application *application);
    void add(Application *application);
    void remove(Application* application);

    QModelIndex findIndex(Application* application);
    void resumeApplication(Application *application);
    QString toString() const;

    Application* findApplicationWithPromptSession(const mir::scene::PromptSession* promptSession);
    Application *findClosingApplication(const QString &inputAppId) const;
    Application *findApplication(MirSurfaceInterface* surface);

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

    friend class Application;
    friend class DBusWindowStack;
};

} // namespace qtmir

Q_DECLARE_METATYPE(qtmir::ApplicationManager*)

#endif // APPLICATIONMANAGER_H
