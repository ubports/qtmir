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

#include <Unity/Application/session_interface.h>
#include <Unity/Application/sessionmodel.h>
#include <Unity/Application/mirsurfacelistmodel.h>
#include <QDebug>

#ifndef QTMIR_FAKE_SESSION_H
#define QTMIR_FAKE_SESSION_H

namespace qtmir {

class FakeSession : public SessionInterface
{
    Q_OBJECT

public:
    FakeSession();
    virtual ~FakeSession();

    QString name() const override;
    unity::shell::application::ApplicationInfoInterface* application() const override;
    MirSurfaceListModel* surfaceList() override;
    MirSurfaceListModel* promptSurfaceList() override;
    SessionModel* childSessions() const override;
    State state() const override;
    bool fullscreen() const override;
    bool live() const override;

    std::shared_ptr<mir::scene::Session> session() const override;

    // For MirSurfaceItem and MirSurfaceManager use

    void registerSurface(MirSurfaceInterface*) override;

    // For Application use

    void setApplication(unity::shell::application::ApplicationInfoInterface* app) override;
    void suspend() override;
    void resume() override;
    void stop() override;
    bool hadSurface() const override;
    bool hasClosingSurfaces() const override;
    bool focused() const override { return false; }

    void close() override;

    bool activeFocus() const override { return false; }

    pid_t pid() const override { return 0; }

    // For SessionManager use

    void addChildSession(SessionInterface*) override;
    void insertChildSession(uint, SessionInterface*) override;
    void removeChildSession(SessionInterface*) override;
    void foreachChildSession(const std::function<void(SessionInterface* session)> &) const override;

    PromptSession activePromptSession() const override;
    void foreachPromptSession(const std::function<void(const PromptSession&)> &) const override;

    void setFullscreen(bool) override;
    void setLive(const bool) override;
    void appendPromptSession(const PromptSession&) override;
    void removePromptSession(const PromptSession&) override;

    // For tests

    void setState(State state);
    void setSession(std::shared_ptr<mir::scene::Session> session);

private:
    unity::shell::application::ApplicationInfoInterface* m_application;
    State m_state;
    std::shared_ptr<mir::scene::Session> m_session;
    MirSurfaceListModel m_surfaceList;
    MirSurfaceListModel m_promptSurfaceList;
    SessionModel *m_childSessions{new SessionModel};
};

} // namespace qtmi

#endif // QTMIR_FAKE_SESSION_H
