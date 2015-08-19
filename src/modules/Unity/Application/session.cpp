/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
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
#include "application.h"
#include "debughelpers.h"
#include "session.h"
#include "mirsurfacemanager.h"
#include "mirsurfaceitem.h"

// mirserver
#include "logging.h"

// mir
#include <mir/scene/session.h>
#include <mir/scene/prompt_session.h>
#include <mir/scene/prompt_session_manager.h>

// Qt
#include <QPainter>
#include <QQmlEngine>

namespace ms = mir::scene;

using unity::shell::application::ApplicationInfoInterface;

namespace qtmir
{

Session::Session(const std::shared_ptr<ms::Session>& session,
                 const std::shared_ptr<ms::PromptSessionManager>& promptSessionManager,
                 QObject *parent)
    : SessionInterface(parent)
    , m_session(session)
    , m_application(nullptr)
    , m_surface(nullptr)
    , m_parentSession(nullptr)
    , m_children(new SessionModel(this))
    , m_fullscreen(false)
    , m_state(State::Starting)
    , m_live(true)
    , m_released(false)
    , m_suspendTimer(new QTimer(this))
    , m_promptSessionManager(promptSessionManager)
{
    qCDebug(QTMIR_SESSIONS) << "Session::Session() " << this->name();

    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    m_suspendTimer->setSingleShot(true);
    connect(m_suspendTimer, &QTimer::timeout, this, &Session::doSuspend);
}

Session::~Session()
{
    qCDebug(QTMIR_SESSIONS) << "Session::~Session() " << name();
    stopPromptSessions();

    QList<SessionInterface*> children(m_children->list());
    for (SessionInterface* child : children) {
        delete child;
    }
    if (m_parentSession) {
        m_parentSession->removeChildSession(this);
    }
    if (m_application) {
        m_application->setSession(nullptr);
    }
    delete m_surface; m_surface = nullptr;
    delete m_children; m_children = nullptr;
}

void Session::doSuspend()
{
    Q_ASSERT(m_state == Session::Suspending);
    if (m_surface) {
        m_surface->stopFrameDropper();
    } else {
        qDebug() << "Application::suspend - no surface to call stopFrameDropper() on!";
    }
    setState(Suspended);
}

void Session::release()
{
    qCDebug(QTMIR_SESSIONS) << "Session::release " << name();

    m_released = true;

    if (m_state == Stopped) {
        deleteLater();
    }
}

QString Session::name() const
{
    return QString::fromStdString(m_session->name());
}

std::shared_ptr<ms::Session> Session::session() const
{
    return m_session;
}

ApplicationInfoInterface* Session::application() const
{
    return m_application;
}

MirSurfaceItemInterface* Session::surface() const
{
    // Only notify QML of surface creation once it has drawn its first frame.
    if (m_surface && m_surface->isFirstFrameDrawn()) {
        return m_surface;
    } else {
        return nullptr;
    }
}

SessionInterface* Session::parentSession() const
{
    return m_parentSession;
}

Session::State Session::state() const
{
    return m_state;
}

void Session::setState(State state) {
    if (state != m_state) {
        m_state = state;
        Q_EMIT stateChanged(m_state);
    }
}

bool Session::fullscreen() const
{
    return m_fullscreen;
}

bool Session::live() const
{
    return m_live;
}

void Session::setApplication(ApplicationInfoInterface* application)
{
    if (m_application == application)
        return;

    m_application = static_cast<Application*>(application);
    Q_EMIT applicationChanged(application);
}

void Session::setSurface(MirSurfaceItemInterface *newSurface)
{
    qCDebug(QTMIR_SESSIONS) << "Session::setSurface - session=" << name() << "surface=" << newSurface;

    if (newSurface == m_surface) {
        return;
    }

    if (m_surface) {
        m_surface->disconnect(this);
        m_surface->setSession(nullptr);
        m_surface->setParent(nullptr);
    }

    MirSurfaceItemInterface *previousSurface = surface();
    m_surface = newSurface;

    if (newSurface) {
        m_surface->setParent(this);
        m_surface->setSession(this);

        connect(newSurface, &MirSurfaceItemInterface::stateChanged,
            this, &Session::updateFullscreenProperty);

        // Only notify QML of surface creation once it has drawn its first frame.
        if (m_surface->isFirstFrameDrawn()) {
            setState(Running);
        } else {
            connect(newSurface, &MirSurfaceItemInterface::firstFrameDrawn,
                    this, &Session::onFirstSurfaceFrameDrawn);
        }
    }

    if (previousSurface != surface()) {
        qCDebug(QTMIR_SESSIONS).nospace() << "Session::surfaceChanged - session=" << this
            << " surface=" << m_surface;
        Q_EMIT surfaceChanged(m_surface);
    }

    updateFullscreenProperty();
}

void Session::onFirstSurfaceFrameDrawn()
{
    qCDebug(QTMIR_SESSIONS).nospace() << "Session::surfaceChanged - session=" << this
        << " surface=" << m_surface;
    Q_EMIT surfaceChanged(m_surface);
    setState(Running);
}

void Session::updateFullscreenProperty()
{
    if (m_surface) {
        setFullscreen(m_surface->state() == MirSurfaceItemInterface::Fullscreen);
    } else {
        // Keep the current value of the fullscreen property until we get a new
        // surface
    }
}

void Session::setFullscreen(bool fullscreen)
{
    qCDebug(QTMIR_SESSIONS) << "Session::setFullscreen - session=" << this << "fullscreen=" << fullscreen;
    if (m_fullscreen != fullscreen) {
        m_fullscreen = fullscreen;
        Q_EMIT fullscreenChanged(m_fullscreen);
    }
}

void Session::suspend()
{
    qCDebug(QTMIR_SESSIONS) << "Session::suspend - session=" << this << "state=" << applicationStateToStr(m_state);
    if (m_state == Running) {
        session()->set_lifecycle_state(mir_lifecycle_state_will_suspend);
        m_suspendTimer->start(1500);

        foreachPromptSession([this](const std::shared_ptr<ms::PromptSession>& promptSession) {
            m_promptSessionManager->suspend_prompt_session(promptSession);
        });

        foreachChildSession([](SessionInterface* session) {
            session->suspend();
        });

        setState(Suspending);
    }
}

void Session::resume()
{
    qCDebug(QTMIR_SESSIONS) << "Session::resume - session=" << this << "state=" << applicationStateToStr(m_state);

    if (m_state == Suspending || m_state == Suspended) {
        doResume();
    }
}

void Session::doResume()
{
    if (m_state == Suspending) {
        Q_ASSERT(m_suspendTimer->isActive());
        m_suspendTimer->stop();
    } else if (m_state == Suspended) {
        Q_ASSERT(m_surface);
        m_surface->startFrameDropper();
    }

    session()->set_lifecycle_state(mir_lifecycle_state_resumed);

    foreachPromptSession([this](const std::shared_ptr<ms::PromptSession>& promptSession) {
        m_promptSessionManager->resume_prompt_session(promptSession);
    });

    foreachChildSession([](SessionInterface* session) {
        session->resume();
    });

    setState(Running);
}

void Session::stop()
{
    if (m_state != Stopped) {
        stopPromptSessions();
        if (m_suspendTimer->isActive())
            m_suspendTimer->stop();
        if (m_surface)
            m_surface->stopFrameDropper();

        foreachChildSession([](SessionInterface* session) {
            session->stop();
        });

        setState(Stopped);
        if (m_released) {
            deleteLater();
        }
    }
}

void Session::setLive(const bool live)
{
    if (m_live != live) {
        m_live = live;
        Q_EMIT liveChanged(m_live);
        if (!live) {
            setState(Stopped);
            if (m_released) {
                deleteLater();
            }
        }
    }
}

void Session::setParentSession(Session* session)
{
    if (m_parentSession == session || session == this)
        return;

    m_parentSession = session;

    Q_EMIT parentSessionChanged(session);
}

void Session::addChildSession(SessionInterface* session)
{
    insertChildSession(m_children->rowCount(), session);
}

void Session::insertChildSession(uint index, SessionInterface* session)
{
    qCDebug(QTMIR_SESSIONS) << "Session::insertChildSession - " << session->name() << " to " << name() << " @  " << index;

    static_cast<Session*>(session)->setParentSession(this);
    m_children->insert(index, session);

    switch (m_state) {
        case Starting:
        case Running:
            session->resume();
            break;
        case Suspending:
        case Suspended:
            session->suspend();
            break;
        case Stopped:
            session->stop();
            break;
    }
}

void Session::removeChildSession(SessionInterface* session)
{
    qCDebug(QTMIR_SESSIONS) << "Session::removeChildSession - " << session->name() << " from " << name();

    if (m_children->contains(session)) {
        m_children->remove(session);
        static_cast<Session*>(session)->setParentSession(nullptr);
    }
}

void Session::foreachChildSession(std::function<void(SessionInterface* session)> f) const
{
    QList<SessionInterface*> children(m_children->list());
    for (SessionInterface* child : children) {
        f(child);
    }
}

SessionModel* Session::childSessions() const
{
    return m_children;
}

void Session::appendPromptSession(const std::shared_ptr<ms::PromptSession>& promptSession)
{
    qCDebug(QTMIR_SESSIONS) << "Session::appendPromptSession session=" << name()
            << "promptSession=" << (promptSession ? promptSession.get() : nullptr);

    m_promptSessions.append(promptSession);
}

void Session::removePromptSession(const std::shared_ptr<ms::PromptSession>& promptSession)
{
    qCDebug(QTMIR_SESSIONS) << "Session::removePromptSession session=" << name()
            << "promptSession=" << (promptSession ? promptSession.get() : nullptr);

    m_promptSessions.removeAll(promptSession);
}

void Session::stopPromptSessions()
{
    QList<SessionInterface*> children(m_children->list());
    for (SessionInterface* child : children) {
        static_cast<Session*>(child)->stopPromptSessions();
    }

    QList<std::shared_ptr<ms::PromptSession>> copy(m_promptSessions);
    QListIterator<std::shared_ptr<ms::PromptSession>> it(copy);
    for ( it.toBack(); it.hasPrevious(); ) {
        std::shared_ptr<ms::PromptSession> promptSession = it.previous();
        qCDebug(QTMIR_SESSIONS) << "Session::stopPromptSessions - promptSession=" << promptSession.get();

        m_promptSessionManager->stop_prompt_session(promptSession);
    }
}

std::shared_ptr<ms::PromptSession> Session::activePromptSession() const
{
    if (m_promptSessions.count() > 0)
        return m_promptSessions.back();
    return nullptr;
}

void Session::foreachPromptSession(std::function<void(const std::shared_ptr<ms::PromptSession>&)> f) const
{
    for (std::shared_ptr<ms::PromptSession> promptSession : m_promptSessions) {
        f(promptSession);
    }
}

} // namespace qtmir
