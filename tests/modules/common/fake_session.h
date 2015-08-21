/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include <Unity/Application/session_interface.h>

#ifndef QTMIR_FAKE_SESSION_H
#define QTMIR_FAKE_SESSION_H

namespace qtmir {

class FakeSession : public SessionInterface
{
    Q_OBJECT

public:
    FakeSession()
        : SessionInterface(0)
        , m_state(Starting)
    {
    }

    // For QML use
    void release() override {}

    QString name() const override { return QString("foo-session"); }
    unity::shell::application::ApplicationInfoInterface* application() const override { return m_application; }
    MirSurfaceItemInterface* surface() const override { return nullptr; }
    SessionInterface* parentSession() const override { return nullptr; }
    SessionModel* childSessions() const override { return nullptr; }
    State state() const override { return m_state; }
    bool fullscreen() const override { return false; }
    bool live() const override { return true; }

    std::shared_ptr<mir::scene::Session> session() const override { return nullptr; }

    // For MirSurfaceItem and MirSurfaceManager use

    void setSurface(MirSurfaceItemInterface*) override {}

    // For Application use

    void setApplication(unity::shell::application::ApplicationInfoInterface* app) override {
        if (m_application != app) {
            m_application = app;
            Q_EMIT applicationChanged(m_application);
        }
    }
    void suspend() override {
        if (m_state == Running) {
            setState(Suspending);
        }
    }
    void resume() override {
        if (m_state == Suspending || m_state == Suspended) {
            setState(Running);
        }
    }
    void stop() override {
        setState(Stopped);
    }

    // For SessionManager use

    void addChildSession(SessionInterface*) override {}
    void insertChildSession(uint, SessionInterface*) override {}
    void removeChildSession(SessionInterface*) override {}
    void foreachChildSession(std::function<void(SessionInterface* session)>) const override {}

    std::shared_ptr<mir::scene::PromptSession> activePromptSession() const override {
        return std::shared_ptr<mir::scene::PromptSession>();
    }
    void foreachPromptSession(std::function<void(const std::shared_ptr<mir::scene::PromptSession>&)>) const override {}

    void setFullscreen(bool) override {}
    void setLive(const bool) override {}
    void appendPromptSession(const std::shared_ptr<mir::scene::PromptSession>&) override {}
    void removePromptSession(const std::shared_ptr<mir::scene::PromptSession>&) override {}

    // For tests

    void setState(State state) {
        if (m_state != state) {
            m_state = state;
            Q_EMIT stateChanged(m_state);
        }
    }

private:
    unity::shell::application::ApplicationInfoInterface* m_application;
    State m_state;
};

} // namespace qtmi

#endif // QTMIR_FAKE_SESSION_H
