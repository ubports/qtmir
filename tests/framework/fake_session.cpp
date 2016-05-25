/*
 * Copyright (C) 2015-2016 Canonical, Ltd.
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

#include "fake_session.h"

namespace qtmir
{

FakeSession::FakeSession()
    : SessionInterface(0)
    , m_application(nullptr)
    , m_state(Starting)
{
}

FakeSession::~FakeSession()
{
    delete m_childSessions;
}

QString FakeSession::name() const { return QString("foo-session"); }

unity::shell::application::ApplicationInfoInterface *FakeSession::application() const { return m_application; }

MirSurfaceListModel* FakeSession::surfaceList() { return &m_surfaceList; }

MirSurfaceListModel* FakeSession::promptSurfaceList() { return &m_promptSurfaceList; }

SessionModel *FakeSession::childSessions() const { return m_childSessions; }

SessionInterface::State FakeSession::state() const { return m_state; }

bool FakeSession::fullscreen() const { return false; }

bool FakeSession::live() const { return true; }

std::shared_ptr<mir::scene::Session> FakeSession::session() const
{
    return m_session;
}

void FakeSession::registerSurface(MirSurfaceInterface *) {}

void FakeSession::setApplication(unity::shell::application::ApplicationInfoInterface *app)
{
    if (m_application != app) {
        m_application = app;
        Q_EMIT applicationChanged(m_application);
    }
}

void FakeSession::suspend()
{
    if (m_state == Running) {
        setState(Suspending);
    }
}

void FakeSession::resume()
{
    if (m_state == Suspending || m_state == Suspended) {
        setState(Running);
    }
}

void FakeSession::stop()
{
    setState(Stopped);
}

bool FakeSession::hasClosingSurfaces() const { return false; }
bool FakeSession::hadSurface() const { return false; }

void FakeSession::close() {}

void FakeSession::addChildSession(SessionInterface *) {}

void FakeSession::insertChildSession(uint, SessionInterface *) {}

void FakeSession::removeChildSession(SessionInterface *) {}

void FakeSession::foreachChildSession(std::function<void (SessionInterface *)>) const {}

std::shared_ptr<mir::scene::PromptSession> FakeSession::activePromptSession() const
{
    return std::shared_ptr<mir::scene::PromptSession>();
}

void FakeSession::foreachPromptSession(std::function<void (const std::shared_ptr<mir::scene::PromptSession> &)>) const {}

void FakeSession::setFullscreen(bool) {}

void FakeSession::setLive(const bool) {}

void FakeSession::appendPromptSession(const std::shared_ptr<mir::scene::PromptSession> &) {}

void FakeSession::removePromptSession(const std::shared_ptr<mir::scene::PromptSession> &) {}

void FakeSession::setState(SessionInterface::State state)
{
    if (m_state != state) {
        m_state = state;
        Q_EMIT stateChanged(m_state);
    }
}

void FakeSession::setSession(std::shared_ptr<mir::scene::Session> session)
{
    m_session = session;
}

} // namespace qtmir
