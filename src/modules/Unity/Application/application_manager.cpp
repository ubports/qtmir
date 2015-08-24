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

// local
#include "application_manager.h"
#include "application.h"
#include "desktopfilereader.h"
#include "dbuswindowstack.h"
#include "session.h"
#include "sharedwakelock.h"
#include "proc_info.h"
#include "taskcontroller.h"
#include "upstart/applicationcontroller.h"
#include "tracepoints.h" // generated from tracepoints.tp
#include "settings.h"

// mirserver
#include "mirserver.h"
#include "nativeinterface.h"
#include "sessionlistener.h"
#include "sessionauthorizer.h"
#include "taskcontroller.h"
#include "logging.h"

// mir
#include <mir/scene/surface.h>
#include <mir/scene/session.h>
#include <mir/graphics/display.h>
#include <mir/graphics/display_buffer.h>
#include <mir/geometry/rectangles.h>

// Qt
#include <QGuiApplication>
#include <QDebug>
#include <QByteArray>
#include <QDir>

// std
#include <csignal>

namespace ms = mir::scene;

Q_LOGGING_CATEGORY(QTMIR_APPLICATIONS, "qtmir.applications")

using namespace unity::shell::application;

namespace qtmir
{

namespace {

// FIXME: To be removed once shell has fully adopted short appIds!!
QString toShortAppIdIfPossible(const QString &appId) {
    QRegExp longAppIdMask("[a-z0-9][a-z0-9+.-]+_[a-zA-Z0-9+.-]+_[0-9][a-zA-Z0-9.+:~-]*");
    if (longAppIdMask.exactMatch(appId)) {
        qWarning() << "WARNING: long App ID encountered:" << appId;
        // input string a long AppId, chop the version string off the end
        QStringList parts = appId.split("_");
        return QString("%1_%2").arg(parts.first()).arg(parts.at(1));
    }
    return appId;
}

void connectToSessionListener(ApplicationManager *manager, SessionListener *listener)
{
    QObject::connect(listener, &SessionListener::sessionStarting,
                     manager, &ApplicationManager::onSessionStarting);
    QObject::connect(listener, &SessionListener::sessionStopping,
                     manager, &ApplicationManager::onSessionStopping);
    QObject::connect(listener, &SessionListener::sessionCreatedSurface,
                     manager, &ApplicationManager::onSessionCreatedSurface);
    QObject::connect(listener, &SessionListener::sessionDestroyingSurface,
                     manager, &ApplicationManager::onSessionDestroyingSurface);
}

void connectToSessionAuthorizer(ApplicationManager *manager, SessionAuthorizer *authorizer)
{
    QObject::connect(authorizer, &SessionAuthorizer::requestAuthorizationForSession,
                     manager, &ApplicationManager::authorizeSession, Qt::BlockingQueuedConnection);
}

void connectToTaskController(ApplicationManager *manager, TaskController *controller)
{
    QObject::connect(controller, &TaskController::processStarting,
                     manager, &ApplicationManager::onProcessStarting);
    QObject::connect(controller, &TaskController::processStopped,
                     manager, &ApplicationManager::onProcessStopped);
    QObject::connect(controller, &TaskController::processSuspended,
                     manager, &ApplicationManager::onProcessSuspended);
    QObject::connect(controller, &TaskController::processFailed,
                     manager, &ApplicationManager::onProcessFailed);
    QObject::connect(controller, &TaskController::focusRequested,
                     manager, &ApplicationManager::onFocusRequested);
    QObject::connect(controller, &TaskController::resumeRequested,
                     manager, &ApplicationManager::onResumeRequested);
}

} // namespace

ApplicationManager* ApplicationManager::Factory::Factory::create()
{
    NativeInterface *nativeInterface = dynamic_cast<NativeInterface*>(QGuiApplication::platformNativeInterface());

    if (!nativeInterface) {
        qCritical() << "ERROR: Unity.Application QML plugin requires use of the 'mirserver' QPA plugin";
        QGuiApplication::quit();
        return nullptr;
    }

    auto mirServer = nativeInterface->m_mirServer;

    SessionListener *sessionListener = static_cast<SessionListener*>(nativeInterface->nativeResourceForIntegration("SessionListener"));
    SessionAuthorizer *sessionAuthorizer = static_cast<SessionAuthorizer*>(nativeInterface->nativeResourceForIntegration("SessionAuthorizer"));

    QSharedPointer<upstart::ApplicationController> appController(new upstart::ApplicationController());
    QSharedPointer<TaskController> taskController(new TaskController(nullptr, appController));
    QSharedPointer<DesktopFileReader::Factory> fileReaderFactory(new DesktopFileReader::Factory());
    QSharedPointer<ProcInfo> procInfo(new ProcInfo());
    QSharedPointer<SharedWakelock> sharedWakelock(new SharedWakelock);
    QSharedPointer<Settings> settings(new Settings());

    // FIXME: We should use a QSharedPointer to wrap this ApplicationManager object, which requires us
    // to use the data() method to pass the raw pointer to the QML engine. However the QML engine appears
    // to take ownership of the object, and deletes it when it wants to. This conflicts with the purpose
    // of the QSharedPointer, and a double-delete results. Trying QQmlEngine::setObjectOwnership on the
    // object no effect, which it should. Need to investigate why.
    ApplicationManager* appManager = new ApplicationManager(
                                             mirServer,
                                             taskController,
                                             sharedWakelock,
                                             fileReaderFactory,
                                             procInfo,
                                             settings
                                         );

    connectToSessionListener(appManager, sessionListener);
    connectToSessionAuthorizer(appManager, sessionAuthorizer);
    connectToTaskController(appManager, taskController.data());

    // Emit signal to notify Upstart that Mir is ready to receive client connections
    // see http://upstart.ubuntu.com/cookbook/#expect-stop
    // FIXME: should not be qtmir's job, instead should notify the user of this library
    // that they should emit this signal, perhaps by posting an event to the
    // QMirServerApplication event loop when it comes up
    if (qgetenv("UNITY_MIR_EMITS_SIGSTOP") == "1") {
        raise(SIGSTOP);
    }

    return appManager;
}


ApplicationManager* ApplicationManager::singleton()
{
    static ApplicationManager* instance;
    if (!instance) {
        Factory appFactory;
        instance = appFactory.create();
    }
    return instance;
}

ApplicationManager::ApplicationManager(
        const QSharedPointer<MirServer>& mirServer,
        const QSharedPointer<TaskController>& taskController,
        const QSharedPointer<SharedWakelock>& sharedWakelock,
        const QSharedPointer<DesktopFileReader::Factory>& desktopFileReaderFactory,
        const QSharedPointer<ProcInfo>& procInfo,
        const QSharedPointer<SettingsInterface>& settings,
        QObject *parent)
    : ApplicationManagerInterface(parent)
    , m_mirServer(mirServer)
    , m_focusedApplication(nullptr)
    , m_dbusWindowStack(new DBusWindowStack(this))
    , m_taskController(taskController)
    , m_desktopFileReaderFactory(desktopFileReaderFactory)
    , m_procInfo(procInfo)
    , m_sharedWakelock(sharedWakelock)
    , m_settings(settings)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::ApplicationManager (this=%p)" << this;
    setObjectName("qtmir::ApplicationManager");

    m_roleNames.insert(RoleSession, "session");
    m_roleNames.insert(RoleFullscreen, "fullscreen");

    if (settings.data()) {
        Application::lifecycleExceptions = m_settings->get("lifecycleExemptAppids").toStringList();
        connect(m_settings.data(), &Settings::changed, this, &ApplicationManager::onSettingsChanged);
    }
}

ApplicationManager::~ApplicationManager()
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::~ApplicationManager";
}

int ApplicationManager::rowCount(const QModelIndex &parent) const
{
    return !parent.isValid() ? m_applications.size() : 0;
}

QVariant ApplicationManager::data(const QModelIndex &index, int role) const
{
    if (index.row() >= 0 && index.row() < m_applications.size()) {
        Application *application = m_applications.at(index.row());
        switch (role) {
            case RoleAppId:
                return QVariant::fromValue(application->appId());
            case RoleName:
                return QVariant::fromValue(application->name());
            case RoleComment:
                return QVariant::fromValue(application->comment());
            case RoleIcon:
                return QVariant::fromValue(application->icon());
            case RoleStage:
                return QVariant::fromValue((int)application->stage());
            case RoleState:
                return QVariant::fromValue((int)application->state());
            case RoleFocused:
                return QVariant::fromValue(application->focused());
            case RoleSession:
                return QVariant::fromValue(application->session());
            case RoleFullscreen:
                return QVariant::fromValue(application->fullscreen());
            default:
                return QVariant();
        }
    } else {
        return QVariant();
    }
}

Application* ApplicationManager::get(int index) const
{
    if (index < 0 || index >= m_applications.count()) {
        return nullptr;
    }
    return m_applications.at(index);
}

Application* ApplicationManager::findApplication(const QString &inputAppId) const
{
    const QString appId = toShortAppIdIfPossible(inputAppId);

    for (Application *app : m_applications) {
        if (app->appId() == appId) {
            return app;
        }
    }
    return nullptr;
}

bool ApplicationManager::requestFocusApplication(const QString &inputAppId)
{
    const QString appId = toShortAppIdIfPossible(inputAppId);

    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::requestFocusApplication - appId=" << appId;
    Application *application = findApplication(appId);

    if (!application) {
        qDebug() << "No such running application with appId=" << appId;
        return false;
    }

    Q_EMIT focusRequested(appId);
    return true;
}

QString ApplicationManager::focusedApplicationId() const
{
    if (m_focusedApplication) {
        return m_focusedApplication->appId();
    } else {
        return QString();
    }
}

bool ApplicationManager::focusApplication(const QString &inputAppId)
{
    const QString appId = toShortAppIdIfPossible(inputAppId);
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::focusApplication - appId=" << appId;
    Application *application = findApplication(appId);

    if (!application) {
        qDebug() << "No such running application with appId=" << appId;
        return false;
    }

    if (m_focusedApplication) {
        m_focusedApplication->setFocused(false);
    }

    m_focusedApplication = application;
    m_focusedApplication->setFocused(true);

    move(m_applications.indexOf(application), 0);
    Q_EMIT focusedApplicationIdChanged();
    m_dbusWindowStack->FocusedWindowChanged(0, application->appId(), application->stage());

    return true;
}

void ApplicationManager::unfocusCurrentApplication()
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::unfocusCurrentApplication";

    m_focusedApplication = nullptr;
    Q_EMIT focusedApplicationIdChanged();
}

/**
 * @brief ApplicationManager::startApplication launches an application identified by an "application id" or appId.
 *
 * Note: due to an implementation detail, appIds come in two forms:
 * * long appId: $(click_package)_$(application)_$(version)
 * * short appId: $(click_package)_$(application)
 * It is expected that the shell uses _only_ short appIds (but long appIds are accepted by this method for legacy
 * reasons - but be warned, this ability will be removed)
 *
 * Unless stated otherwise, we always use short appIds in this API.
 *
 * @param inputAppId AppId of application to launch (long appId supported)
 * @param arguments Command line arguments to pass to the application to be launched
 * @return Pointer to Application object representing the launched process. If process already running, return nullptr
 */
Application* ApplicationManager::startApplication(const QString &appId,
                                                  const QStringList &arguments)
{
    return startApplication(appId, NoFlag, arguments);
}

Application *ApplicationManager::startApplication(const QString &inputAppId, ExecFlags flags,
                                                  const QStringList &arguments)
{
    tracepoint(qtmir, startApplication);
    QString appId = toShortAppIdIfPossible(inputAppId);
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::startApplication - this=" << this << "appId" << qPrintable(appId);

    Application *application = findApplication(appId);
    if (application) {
        qWarning() << "ApplicationManager::startApplication - application appId=" << appId << " already exists";
        return nullptr;
    }

    if (!m_taskController->start(appId, arguments)) {
        qWarning() << "Upstart failed to start application with appId" << appId;
        return nullptr;
    }

    // The TaskController may synchroneously callback onProcessStarting, so check if application already added
    application = findApplication(appId);
    if (application) {
        application->setArguments(arguments);
    } else {
        application = new Application(
                    m_sharedWakelock,
                    m_desktopFileReaderFactory->createInstance(appId, m_taskController->findDesktopFileForAppId(appId)),
                    arguments,
                    this);

        if (!application->isValid()) {
            qWarning() << "Unable to instantiate application with appId" << appId;
            return nullptr;
        }

        // override stage if necessary
        if (application->stage() == Application::SideStage && flags.testFlag(ApplicationManager::ForceMainStage)) {
            application->setStage(Application::MainStage);
        }

        add(application);
    }
    return application;
}

void ApplicationManager::onProcessStarting(const QString &appId)
{
    tracepoint(qtmir, onProcessStarting);
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::onProcessStarting - appId=" << appId;

    Application *application = findApplication(appId);
    if (!application) { // then shell did not start this application, so ubuntu-app-launch must have - add to list
        application = new Application(
                    m_sharedWakelock,
                    m_desktopFileReaderFactory->createInstance(appId, m_taskController->findDesktopFileForAppId(appId)),
                    QStringList(),
                    this);

        if (!application->isValid()) {
            qWarning() << "Unable to instantiate application with appId" << appId;
            return;
        }

        add(application);
        Q_EMIT focusRequested(appId);
    }
    else {
        if (application->state() == Application::Stopped) {
            // url-dispatcher can relaunch apps which have been OOM-killed - AppMan must accept the newly spawned
            // application and focus it immediately (as user expects app to still be running).
            qCDebug(QTMIR_APPLICATIONS) << "Stopped application appId=" << appId << "is being resumed externally";
            Q_EMIT focusRequested(appId);
        } else {
            qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::onProcessStarting application already found with appId"
                                        << appId;
        }
    }
    application->setProcessState(Application::ProcessRunning);
}

/**
 * @brief ApplicationManager::stopApplication - stop a running application and remove from list
 * @param inputAppId
 * @return True if running application was stopped, false if application did not exist or could not be stopped
 */
bool ApplicationManager::stopApplication(const QString &inputAppId)
{
    const QString appId = toShortAppIdIfPossible(inputAppId);
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::stopApplication - appId=" << appId;

    Application *application = findApplication(appId);
    if (!application) {
        qCritical() << "No such running application with appId" << appId;
        return false;
    }

    remove(application);

    bool result = m_taskController->stop(application->longAppId());

    if (!result && application->pid() > 0) {
        qWarning() << "FAILED to ask Upstart to stop application with appId" << appId
                   << "Sending SIGTERM to process:" << application->pid();
        kill(application->pid(), SIGTERM);
        result = true;
    }

    delete application;
    return result;
}

void ApplicationManager::onProcessFailed(const QString &appId, const bool duringStartup)
{
    // Applications fail if they fail to launch, crash or are killed.

    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::onProcessFailed - appId=" << appId << "duringStartup=" << duringStartup;

    Application *application = findApplication(appId);
    if (!application) {
        qWarning() << "ApplicationManager::onProcessFailed - upstart reports failure of application" << appId
                   << "that AppManager is not managing";
        return;
    }

    Q_UNUSED(duringStartup); // FIXME(greyback) upstart reports app that fully started up & crashes as failing during startup??
    application->setProcessState(Application::ProcessFailed);
    application->setPid(0);
}

void ApplicationManager::onProcessStopped(const QString &appId)
{
    tracepoint(qtmir, onProcessStopped);
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::onProcessStopped - appId=" << appId;
    Application *application = findApplication(appId);

    if (!application) {
        qDebug() << "ApplicationManager::onProcessStopped reports stop of appId=" << appId
                 << "which AppMan is not managing, ignoring the event";
        return;
    }

    // if an application gets killed, onProcessFailed is called first, followed by onProcessStopped.
    // we don't want to override what onProcessFailed already set.
    if (application->processState() != Application::ProcessFailed) {
        application->setProcessState(Application::ProcessStopped);
        application->setPid(0);
    }
}

void ApplicationManager::onProcessSuspended(const QString &appId)
{
    Application *application = findApplication(appId);
    if (application) {
        application->setProcessState(Application::ProcessSuspended);
    }
}

void ApplicationManager::onFocusRequested(const QString& appId)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::onFocusRequested - appId=" << appId;

    Q_EMIT focusRequested(appId);
}

void ApplicationManager::onResumeRequested(const QString& appId)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::onResumeRequested - appId=" << appId;

    Application *application = findApplication(appId);

    if (!application) {
        qCritical() << "ApplicationManager::onResumeRequested: No such running application" << appId;
        return;
    }

    // We interpret this as a focus request for a suspended app.
    // Shell will have this app resumed if it complies with the focus request
    if (application->state() == Application::Suspended) {
        Q_EMIT focusRequested(appId);
    }
}

void ApplicationManager::onAppDataChanged(const int role)
{
    if (sender()) {
        Application *application = static_cast<Application*>(sender());
        QModelIndex appIndex = findIndex(application);
        Q_EMIT dataChanged(appIndex, appIndex, QVector<int>() << role);

        qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::onAppDataChanged: Received " << m_roleNames[role] << " update" <<  application->appId();
    } else {
        qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::onAppDataChanged: Received " << m_roleNames[role] << " signal but application has disappeard.";
    }
}

void ApplicationManager::onSettingsChanged(const QString &key)
{
    if (key == "lifecycleExemptAppids") {
        Application::lifecycleExceptions = m_settings->get("lifecycleExemptAppids").toStringList();
    }
}

void ApplicationManager::authorizeSession(const quint64 pid, bool &authorized)
{
    tracepoint(qtmir, authorizeSession);
    authorized = false; //to be proven wrong

    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::authorizeSession - pid=" << pid;

    for (Application *app : m_applications) {
        if (app->state() == Application::Starting) {
            tracepoint(qtmir, appIdHasProcessId_start);
            if (m_taskController->appIdHasProcessId(app->appId(), pid)) {
                app->setPid(pid);
                authorized = true;
                tracepoint(qtmir, appIdHasProcessId_end, 1); //found
                return;
            }
            tracepoint(qtmir, appIdHasProcessId_end, 0); // not found
        }
    }

    /*
     * Hack: Allow applications to be launched without being managed by upstart, where AppManager
     * itself manages processes executed with a "--desktop_file_hint=/path/to/desktopFile.desktop"
     * parameter attached. This exists until ubuntu-app-launch can notify shell any application is
     * and so shell should allow it. Also reads the --stage parameter to determine the desired stage
     */
    std::unique_ptr<ProcInfo::CommandLine> info = m_procInfo->commandLine(pid);
    if (!info) {
        qWarning() << "ApplicationManager REJECTED connection from app with pid" << pid
                   << "as unable to read the process command line";
        return;
    }

    if (info->startsWith("maliit-server") || info->contains("qt5/libexec/QtWebProcess")) {
        authorized = true;
        return;
    }

    const QString desktopFileName = info->getParameter("--desktop_file_hint=");

    if (desktopFileName.isNull()) {
        qCritical() << "ApplicationManager REJECTED connection from app with pid" << pid
                    << "as it was not launched by upstart, and no desktop_file_hint is specified";
        return;
    }

    qCDebug(QTMIR_APPLICATIONS) << "Process supplied desktop_file_hint, loading:" << desktopFileName;

    // Guess appId from the desktop file hint
    const QString appId = toShortAppIdIfPossible(desktopFileName.split('/').last().remove(QRegExp(".desktop$")));

    // FIXME: right now we support --desktop_file_hint=appId for historical reasons. So let's try that in
    // case we didn't get an existing .desktop file path
    DesktopFileReader* desktopData;
    if (QFileInfo::exists(desktopFileName)) {
        desktopData = m_desktopFileReaderFactory->createInstance(appId, QFileInfo(desktopFileName));
    } else {
        qCDebug(QTMIR_APPLICATIONS) << "Unable to find file:" << desktopFileName
                                    << "so will search standard paths for one named" << appId << ".desktop";
        desktopData = m_desktopFileReaderFactory->createInstance(appId, m_taskController->findDesktopFileForAppId(appId));
    }

    if (!desktopData->loaded()) {
        delete desktopData;
        qCritical() << "ApplicationManager REJECTED connection from app with pid" << pid
                    << "as the file specified by the desktop_file_hint argument could not be opened";
        return;
    }

    // some naughty applications use a script to launch the actual application. Check for the
    // case where shell actually launched the script.
    Application *application = findApplication(desktopData->appId());
    if (application && application->state() == Application::Starting) {
        qCDebug(QTMIR_APPLICATIONS) << "Process with pid" << pid << "appeared, attaching to existing entry"
                                    << "in application list with appId:" << application->appId();
        delete desktopData;
        application->setPid(pid);
        authorized = true;
        return;
    }

    // if stage supplied in CLI, fetch that
    Application::Stage stage = Application::MainStage;
    QString stageParam = info->getParameter("--stage_hint=");

    if (stageParam == "side_stage") {
        stage = Application::SideStage;
    }

    qCDebug(QTMIR_APPLICATIONS) << "New process with pid" << pid << "appeared, adding new application to the"
                                << "application list with appId:" << desktopData->appId();

    QStringList arguments(info->asStringList());
    application = new Application(
        m_sharedWakelock,
        desktopData,
        arguments,
        this);
    application->setPid(pid);
    application->setStage(stage);
    add(application);
    authorized = true;
}

void ApplicationManager::onSessionStarting(std::shared_ptr<ms::Session> const& session)
{
    Q_UNUSED(session);
}

void ApplicationManager::onSessionStopping(std::shared_ptr<ms::Session> const& session)
{
    Application* application = findApplicationWithSession(session);
    if (application) {
        m_dbusWindowStack->WindowDestroyed(0, application->appId());
    }
}

void ApplicationManager::onSessionCreatedSurface(ms::Session const* session,
                                               std::shared_ptr<ms::Surface> const& surface)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::onSessionCreatedSurface - sessionName=" << session->name().c_str();
    Q_UNUSED(surface);

    Application* application = findApplicationWithSession(session);
    if (application) {
        m_dbusWindowStack->WindowCreated(0, application->appId());
    }
}

void ApplicationManager::onSessionDestroyingSurface(ms::Session const* session,
                                                    std::shared_ptr<ms::Surface> const& surface)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::onSessionDestroyingSurface - sessionName=" << session->name().c_str();
    Q_UNUSED(surface);

    Application* application = findApplicationWithSession(session);
    if (application && application->state() == Application::Running) {
        // If app in Running state but it destroys its surface, it's probably shutting down,
        // in which case, we can preempt it and remove it from the App list immediately.
        // FIXME: this is not desktop application friendly, but resolves issue where trust-prompt
        // helpers take a long time to shut down, but destroys their surface quickly.
        remove(application);
        application->deleteLater();
    }
}

Application* ApplicationManager::findApplicationWithSession(const std::shared_ptr<ms::Session> &session)
{
    return findApplicationWithSession(session.get());
}

Application* ApplicationManager::findApplicationWithSession(const ms::Session *session)
{
    if (!session)
        return nullptr;
    return findApplicationWithPid(session->process_id());
}

Application* ApplicationManager::findApplicationWithPid(const qint64 pid)
{
    if (pid <= 0)
        return nullptr;

    for (Application *app : m_applications) {
        if (app->m_pid == pid) {
            return app;
        }
    }
    return nullptr;
}

void ApplicationManager::add(Application* application)
{
    Q_ASSERT(application != nullptr);
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::add - appId=" << application->appId();

    connect(application, &Application::fullscreenChanged, this, [this](bool) { onAppDataChanged(RoleFullscreen); });
    connect(application, &Application::focusedChanged, this, [this](bool) { onAppDataChanged(RoleFocused); });
    connect(application, &Application::stateChanged, this, [this](Application::State) { onAppDataChanged(RoleState); });
    connect(application, &Application::stageChanged, this, [this](Application::Stage) { onAppDataChanged(RoleStage); });

    QString appId = application->appId();
    QString longAppId = application->longAppId();
    QStringList arguments = application->arguments();

    // The connection is queued as a workaround an issue in the PhoneStage animation that
    // happens when you tap on a killed app in the spread to bring it to foreground, causing
    // a Application::respawn() to take place.
    // In any case, it seems like in general QML works better when don't do too many things
    // in the same event loop iteration.
    connect(application, &Application::startProcessRequested,
            this, [=]() { m_taskController->start(appId, arguments); },
            Qt::QueuedConnection);

    connect(application, &Application::suspendProcessRequested, this, [=]() { m_taskController->suspend(longAppId); } );
    connect(application, &Application::resumeProcessRequested, this, [=]() { m_taskController->resume(longAppId); } );

    connect(application, &Application::stopped, this, [=]() {
        remove(application);
        application->deleteLater();
    });


    beginInsertRows(QModelIndex(), m_applications.count(), m_applications.count());
    m_applications.append(application);
    endInsertRows();
    Q_EMIT countChanged();
    Q_EMIT applicationAdded(application->appId());
    if (m_applications.size() == 1) {
        Q_EMIT emptyChanged();
    }
}

void ApplicationManager::remove(Application *application)
{
    Q_ASSERT(application != nullptr);
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::remove - appId=" << application->appId();

    application->disconnect(this);

    int i = m_applications.indexOf(application);
    if (i != -1) {
        beginRemoveRows(QModelIndex(), i, i);
        m_applications.removeAt(i);
        endRemoveRows();
        Q_EMIT applicationRemoved(application->appId());
        Q_EMIT countChanged();
        if (i == 0) {
            Q_EMIT emptyChanged();
        }
    }

    if (application == m_focusedApplication) {
        m_focusedApplication = nullptr;
        Q_EMIT focusedApplicationIdChanged();
    }
}

void ApplicationManager::move(int from, int to) {
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::move - from=" << from << "to=" << to;
    if (from == to) return;

    if (from >= 0 && from < m_applications.size() && to >= 0 && to < m_applications.size()) {
        QModelIndex parent;
        /* When moving an item down, the destination index needs to be incremented
           by one, as explained in the documentation:
           http://qt-project.org/doc/qt-5.0/qtcore/qabstractitemmodel.html#beginMoveRows */

        beginMoveRows(parent, from, from, parent, to + (to > from ? 1 : 0));
        m_applications.move(from, to);
        endMoveRows();
    }
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::move after " << toString();
}

QModelIndex ApplicationManager::findIndex(Application* application)
{
    for (int i = 0; i < m_applications.size(); ++i) {
        if (m_applications.at(i) == application) {
            return index(i);
        }
    }

    return QModelIndex();
}

QString ApplicationManager::toString() const
{
    QString result;
    for (int i = 0; i < m_applications.count(); ++i) {
        if (i > 0) {
            result.append(",");
        }
        result.append(m_applications.at(i)->appId());
    }
    return result;
}

} // namespace qtmir
