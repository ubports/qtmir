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

#ifndef SESSION_INTERFACE_H
#define SESSION_INTERFACE_H

#include <functional>
#include <memory>

// Unity API
#include <unity/shell/application/ApplicationInfoInterface.h>

// local
#include "sessionmodel.h"

namespace mir {
    namespace scene {
        class Session;
        class PromptSession;
    }
}

namespace qtmir {

class MirSurfaceItemInterface;

class SessionInterface : public QObject {
    Q_OBJECT
    Q_PROPERTY(MirSurfaceItemInterface* surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(unity::shell::application::ApplicationInfoInterface* application READ application NOTIFY applicationChanged DESIGNABLE false)
    Q_PROPERTY(SessionInterface* parentSession READ parentSession NOTIFY parentSessionChanged DESIGNABLE false)
    Q_PROPERTY(SessionModel* childSessions READ childSessions DESIGNABLE false CONSTANT)
    Q_PROPERTY(bool fullscreen READ fullscreen NOTIFY fullscreenChanged)
    Q_PROPERTY(bool live READ live NOTIFY liveChanged)
public:
    SessionInterface(QObject *parent = 0) : QObject(parent) {}
    virtual ~SessionInterface() {}

    enum State {
        Starting,
        Running,
        Suspending,
        Suspended,
        Stopped
    };

    Q_INVOKABLE virtual void release() = 0;

    //getters
    virtual QString name() const = 0;
    virtual unity::shell::application::ApplicationInfoInterface* application() const = 0;
    virtual MirSurfaceItemInterface* surface() const = 0;
    virtual SessionInterface* parentSession() const = 0;
    virtual SessionModel* childSessions() const = 0;
    virtual State state() const = 0;
    virtual bool fullscreen() const = 0;
    virtual bool live() const = 0;

    virtual std::shared_ptr<mir::scene::Session> session() const = 0;

    // For MirSurfaceItem and MirSurfaceManager use

    virtual void setSurface(MirSurfaceItemInterface* surface) = 0;

    // For Application use

    virtual void setApplication(unity::shell::application::ApplicationInfoInterface* item) = 0;
    virtual void suspend() = 0;
    virtual void resume() = 0;
    virtual void stop() = 0;

    // For SessionManager use

    virtual void addChildSession(SessionInterface* session) = 0;
    virtual void insertChildSession(uint index, SessionInterface* session) = 0;
    virtual void removeChildSession(SessionInterface* session) = 0;
    virtual void foreachChildSession(std::function<void(SessionInterface* session)> f) const = 0;

    virtual std::shared_ptr<mir::scene::PromptSession> activePromptSession() const = 0;
    virtual void foreachPromptSession(std::function<void(const std::shared_ptr<mir::scene::PromptSession>&)> f) const = 0;

    virtual void setFullscreen(bool fullscreen) = 0;
    virtual void setLive(const bool) = 0;
    virtual void appendPromptSession(const std::shared_ptr<mir::scene::PromptSession>& session) = 0;
    virtual void removePromptSession(const std::shared_ptr<mir::scene::PromptSession>& session) = 0;

Q_SIGNALS:
    void surfaceChanged(MirSurfaceItemInterface*);
    void parentSessionChanged(SessionInterface*);
    void applicationChanged(unity::shell::application::ApplicationInfoInterface* application);
    void stateChanged(State state);
    void fullscreenChanged(bool fullscreen);
    void liveChanged(bool live);
};

} // namespace qtmir

Q_DECLARE_METATYPE(qtmir::SessionInterface*)

#endif // SESSION_INTERFACE_H
