/*
 * Copyright (C) 2017 Canonical, Ltd.
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
#include "taskcontroller.h"
#include "session.h"

#include <QGuiApplication>

// QPA mirserver
#include "logging.h"
#include "nativeinterface.h"
#include "promptsessionlistener.h"
#include "promptsession.h"
#include "qtmir/sessionauthorizer.h"

// common
#include "qtmir/appnotifier.h"

#define DEBUG_MSG qCDebug(QTMIR_SESSIONS).nospace() << "TaskController::" << __func__

using namespace qtmir;

TaskController::TaskController(QObject *parent)
    : QObject(parent)
{
    NativeInterface *nativeInterface = dynamic_cast<NativeInterface*>(QGuiApplication::platformNativeInterface());

    if (!nativeInterface) {
        qFatal("ERROR: Unity.Application QML plugin requires use of the 'mirserver' QPA plugin");
    }

    m_promptSessionManager = nativeInterface->thePromptSessionManager();

    auto appNotifier = static_cast<AppNotifier*>(nativeInterface->nativeResourceForIntegration("AppNotifier"));
    connectToAppNotifier(appNotifier);

    auto promptSessionListener = static_cast<PromptSessionListener*>(nativeInterface->nativeResourceForIntegration("PromptSessionListener"));
    connectToPromptSessionListener(promptSessionListener);

    auto sessionAuthorizer = static_cast<SessionAuthorizer*>(nativeInterface->nativeResourceForIntegration("SessionAuthorizer"));
    QObject::connect(sessionAuthorizer, &SessionAuthorizer::requestAuthorizationForSession,
                     this, &TaskController::onAuthorizationForSessionRequested, Qt::BlockingQueuedConnection);
}

TaskController::TaskController(std::shared_ptr<PromptSessionManager> &promptSessionManager, QObject *parent)
    : QObject(parent)
    , m_promptSessionManager(promptSessionManager)
{
}

void TaskController::onSessionStarting(const miral::ApplicationInfo &appInfo)
{
    DEBUG_MSG << " - sessionName=" <<  appInfo.name().c_str();

    const auto &session = appInfo.application();
    Session* qmlSession = new Session(session, m_promptSessionManager);
    m_sessionList.prepend(qmlSession);

    // need to remove if we've destroyed outside
    connect(qmlSession, &Session::destroyed, this, [&](QObject *item) {
        auto sessionToRemove = static_cast<Session*>(item);
        m_sessionList.removeAll(sessionToRemove);
    });

    Q_EMIT sessionStarting(qmlSession);
}

void TaskController::onSessionStopping(const miral::ApplicationInfo &appInfo)
{
    DEBUG_MSG << " - sessionName=" << appInfo.name().c_str();

    SessionInterface* qmlSession = findSession(appInfo.application().get());
    if (!qmlSession) return;

    m_sessionList.removeAll(qmlSession);

    qmlSession->setLive(false);
}

void TaskController::onPromptSessionStarting(const qtmir::PromptSession &promptSession)
{
    DEBUG_MSG << " - promptSession=" << promptSession.get();

    std::shared_ptr<mir::scene::Session> appSession = m_promptSessionManager->applicationFor(promptSession);
    SessionInterface *qmlAppSession = findSession(appSession.get());
    if (qmlAppSession) {
        m_mirPromptToSessionHash[promptSession.get()] = qmlAppSession;
        qmlAppSession->appendPromptSession(promptSession);
    } else {
        DEBUG_MSG << " - could not find app session for prompt session";
    }
}

void TaskController::onPromptSessionStopping(const qtmir::PromptSession &promptSession)
{
    DEBUG_MSG << " - promptSession=" << promptSession.get();

    for (SessionInterface *qmlSession : m_sessionList) {
        qmlSession->removePromptSession(promptSession);
    }
    m_mirPromptToSessionHash.remove(promptSession.get());
}

void TaskController::onPromptProviderAdded(const qtmir::PromptSession &promptSession,
                                           const miral::Application &promptProvider)
{
    DEBUG_MSG << " - promptSession=" << promptSession.get() << " promptProvider=" << promptProvider.get();

    SessionInterface* qmlAppSession = m_mirPromptToSessionHash.value(promptSession.get(), nullptr);
    if (!qmlAppSession) {
        DEBUG_MSG << " - could not find session item for app session";
        return;
    }

    SessionInterface* qmlPromptProvider = findSession(promptProvider.get());
    if (!qmlPromptProvider) {
        DEBUG_MSG << " - could not find session item for provider session";
        return;
    }

    qmlAppSession->addChildSession(qmlPromptProvider);
}

void TaskController::onPromptProviderRemoved(const qtmir::PromptSession &promptSession,
                                             const miral::Application &promptProvider)
{
    DEBUG_MSG << " - promptSession=" << promptSession.get() << " promptProvider=" << promptProvider.get();

    SessionInterface* qmlPromptProvider = findSession(promptProvider.get());
    if (!qmlPromptProvider) {
        DEBUG_MSG << " - could not find session item for provider session";
        return;
    }
    qmlPromptProvider->setLive(false);
}

SessionInterface *TaskController::findSession(const mir::scene::Session* session) const
{
    if (!session) return nullptr;

    for (SessionInterface* child : m_sessionList) {
        if (child->session().get() == session)
            return child;
    }
    return nullptr;
}

void TaskController::onAuthorizationForSessionRequested(const pid_t &pid, bool &authorized)
{
    Q_EMIT authorizationRequestedForSession(pid, authorized);
}

void TaskController::connectToAppNotifier(AppNotifier *appNotifier)
{
    QObject::connect(appNotifier, &AppNotifier::appAdded,
                     this, &TaskController::onSessionStarting);
    QObject::connect(appNotifier, &AppNotifier::appRemoved,
                     this, &TaskController::onSessionStopping);
}

void TaskController::connectToPromptSessionListener(PromptSessionListener *listener)
{
    QObject::connect(listener, &PromptSessionListener::promptSessionStarting,
                     this, &TaskController::onPromptSessionStarting);
    QObject::connect(listener, &PromptSessionListener::promptSessionStopping,
                     this, &TaskController::onPromptSessionStopping);
    QObject::connect(listener, &PromptSessionListener::promptProviderAdded,
                     this, &TaskController::onPromptProviderAdded);
    QObject::connect(listener, &PromptSessionListener::promptProviderRemoved,
                     this, &TaskController::onPromptProviderRemoved);
}
