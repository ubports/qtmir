/*
 * Copyright (C) 2014-2016 Canonical, Ltd.
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
#include <QGuiApplication>

// local
#include "application_manager.h"
#include "debughelpers.h"
#include "sessionmanager.h"

// QPA mirserver
#include "logging.h"
#include "nativeinterface.h"
#include "promptsessionlistener.h"
#include "promptsession.h"
#include "qtmir/appnotifier.h"


namespace qtmir {

SessionManager *SessionManager::the_session_manager = nullptr;


void connectToAppNotifier(SessionManager *manager, AppNotifier *appNotifier)
{
    QObject::connect(appNotifier, &AppNotifier::appAdded,
                     manager, &SessionManager::onSessionStarting);
    QObject::connect(appNotifier, &AppNotifier::appRemoved,
                     manager, &SessionManager::onSessionStopping);
}

void connectToPromptSessionListener(SessionManager * manager, PromptSessionListener * listener)
{
    QObject::connect(listener, &PromptSessionListener::promptSessionStarting,
                     manager, &SessionManager::onPromptSessionStarting);
    QObject::connect(listener, &PromptSessionListener::promptSessionStopping,
                     manager, &SessionManager::onPromptSessionStopping);
    QObject::connect(listener, &PromptSessionListener::promptProviderAdded,
                     manager, &SessionManager::onPromptProviderAdded);
    QObject::connect(listener, &PromptSessionListener::promptProviderRemoved,
                     manager, &SessionManager::onPromptProviderRemoved);
}

SessionManager* SessionManager::singleton()
{
    if (!the_session_manager) {

        NativeInterface *nativeInterface = dynamic_cast<NativeInterface*>(QGuiApplication::platformNativeInterface());

        if (!nativeInterface) {
            qCritical("ERROR: Unity.Application QML plugin requires use of the 'mirserver' QPA plugin");
            QGuiApplication::quit();
            return nullptr;
        }

        auto appNotifier = static_cast<AppNotifier*>(nativeInterface->nativeResourceForIntegration("AppNotifier"));
        PromptSessionListener *promptSessionListener = static_cast<PromptSessionListener*>(nativeInterface->nativeResourceForIntegration("PromptSessionListener"));

        the_session_manager = new SessionManager(nativeInterface->thePromptSessionManager(), ApplicationManager::singleton());

        connectToAppNotifier(the_session_manager, appNotifier);
        connectToPromptSessionListener(the_session_manager, promptSessionListener);
    }
    return the_session_manager;
}

SessionManager::SessionManager(
        const std::shared_ptr<qtmir::PromptSessionManager>& promptSessionManager,
        ApplicationManager* applicationManager,
        QObject *parent)
    : SessionModel(parent)
    , m_promptSessionManager(promptSessionManager)
    , m_applicationManager(applicationManager)
{
    qCDebug(QTMIR_SESSIONS) << "SessionManager::SessionManager - this=" << this;
    setObjectName(QStringLiteral("qtmir::SessionManager"));
}

SessionManager::~SessionManager()
{
    qCDebug(QTMIR_SESSIONS) << "SessionManager::~SessionManager - this=" << this;
}

SessionInterface *SessionManager::findSession(const mir::scene::Session* session) const
{
    if (!session) return nullptr;

    for (SessionInterface* child : list()) {
        if (child->session().get() == session)
            return child;
    }
    return nullptr;
}

void SessionManager::onSessionStarting(const miral::ApplicationInfo &appInfo)
{
    qCDebug(QTMIR_SESSIONS) << "SessionManager::onSessionStarting - sessionName=" <<  appInfo.name().c_str();

    const auto &session = appInfo.application();
    Session* qmlSession = new Session(session, m_promptSessionManager);
    insert(0, qmlSession);

    Application* application = m_applicationManager->findApplicationWithSession(session);
    if (application && application->state() != Application::Running) {
        application->setSession(qmlSession);
    }
    // need to remove if we've destroyed outside
    connect(qmlSession, &Session::destroyed, this, [&](QObject *item) {
        auto sessionToRemove = static_cast<Session*>(item);
        remove(sessionToRemove);
    });

    Q_EMIT sessionStarting(qmlSession);
}

void SessionManager::onSessionStopping(const miral::ApplicationInfo &appInfo)
{
    qCDebug(QTMIR_SESSIONS) << "SessionManager::onSessionStopping - sessionName=" << appInfo.name().c_str();

    SessionInterface* qmlSession = findSession(appInfo.application().get());
    if (!qmlSession) return;

    remove(qmlSession);

    qmlSession->setLive(false);
    Q_EMIT sessionStopping(qmlSession);
}

void SessionManager::onPromptSessionStarting(const qtmir::PromptSession &promptSession)
{
    qCDebug(QTMIR_SESSIONS) << "SessionManager::onPromptSessionStarting - promptSession=" << promptSession.get();

    std::shared_ptr<mir::scene::Session> appSession = m_promptSessionManager->applicationFor(promptSession);
    SessionInterface *qmlAppSession = findSession(appSession.get());
    if (qmlAppSession) {
        m_mirPromptToSessionHash[promptSession.get()] = qmlAppSession;
        qmlAppSession->appendPromptSession(promptSession);
    } else {
        qCDebug(QTMIR_SESSIONS) << "SessionManager::onPromptSessionStarting - could not find app session for prompt session";
    }
}

void SessionManager::onPromptSessionStopping(const qtmir::PromptSession &promptSession)
{
    qCDebug(QTMIR_SESSIONS) << "SessionManager::onPromptSessionStopping - promptSession=" << promptSession.get();

    for (SessionInterface *qmlSession : this->list()) {
        qmlSession->removePromptSession(promptSession);
    }
    m_mirPromptToSessionHash.remove(promptSession.get());
}

void SessionManager::onPromptProviderAdded(const qtmir::PromptSession &promptSession,
                                              const std::shared_ptr<mir::scene::Session> &promptProvider)
{
    qCDebug(QTMIR_SESSIONS) << "SessionManager::onPromptProviderAdded - promptSession=" << promptSession.get() << " promptProvider=" << promptProvider.get();

    SessionInterface* qmlAppSession = m_mirPromptToSessionHash.value(promptSession.get(), nullptr);
    if (!qmlAppSession) {
        qCDebug(QTMIR_SESSIONS) << "SessionManager::onPromptProviderAdded - could not find session item for app session";
        return;
    }

    SessionInterface* qmlPromptProvider = findSession(promptProvider.get());
    if (!qmlPromptProvider) {
        qCDebug(QTMIR_SESSIONS) << "SessionManager::onPromptProviderAdded - could not find session item for provider session";
        return;
    }

    qmlAppSession->addChildSession(qmlPromptProvider);
}

void SessionManager::onPromptProviderRemoved(const qtmir::PromptSession &promptSession,
                                                const std::shared_ptr<mir::scene::Session> &promptProvider)
{
    qCDebug(QTMIR_SESSIONS) << "SessionManager::onPromptProviderRemoved - promptSession=" << promptSession.get() << " promptProvider=" << promptProvider.get();

    SessionInterface* qmlPromptProvider = findSession(promptProvider.get());
    if (!qmlPromptProvider) {
        qCDebug(QTMIR_SESSIONS) << "SessionManager::onPromptProviderAdded - could not find session item for provider session";
        return;
    }
    qmlPromptProvider->setLive(false);
}

} // namespace qtmir
