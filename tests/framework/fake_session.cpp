#include "fake_session.h"

qtmir::FakeSession::FakeSession()
    : SessionInterface(0)
    , m_state(Starting)
{
}

qtmir::FakeSession::~FakeSession()
{
}

void qtmir::FakeSession::release() {}

QString qtmir::FakeSession::name() const { return QString("foo-session"); }

unity::shell::application::ApplicationInfoInterface *qtmir::FakeSession::application() const { return m_application; }

qtmir::MirSurfaceInterface *qtmir::FakeSession::surface() const { return nullptr; }

qtmir::SessionInterface *qtmir::FakeSession::parentSession() const { return nullptr; }

qtmir::SessionModel *qtmir::FakeSession::childSessions() const { return nullptr; }

qtmir::SessionInterface::State qtmir::FakeSession::state() const { return m_state; }

bool qtmir::FakeSession::fullscreen() const { return false; }

bool qtmir::FakeSession::live() const { return true; }

std::shared_ptr<mir::scene::Session> qtmir::FakeSession::session() const { return nullptr; }

void qtmir::FakeSession::setSurface(qtmir::MirSurfaceInterface *) {}

void qtmir::FakeSession::setApplication(unity::shell::application::ApplicationInfoInterface *app) {
    if (m_application != app) {
        m_application = app;
        Q_EMIT applicationChanged(m_application);
    }
}

void qtmir::FakeSession::suspend() {
    if (m_state == Running) {
        setState(Suspending);
    }
}

void qtmir::FakeSession::resume() {
    if (m_state == Suspending || m_state == Suspended) {
        setState(Running);
    }
}

void qtmir::FakeSession::stop() {
    setState(Stopped);
}

void qtmir::FakeSession::addChildSession(qtmir::SessionInterface *) {}

void qtmir::FakeSession::insertChildSession(uint, qtmir::SessionInterface *) {}

void qtmir::FakeSession::removeChildSession(qtmir::SessionInterface *) {}

void qtmir::FakeSession::foreachChildSession(std::function<void (qtmir::SessionInterface *)>) const {}

std::shared_ptr<mir::scene::PromptSession> qtmir::FakeSession::activePromptSession() const {
    return std::shared_ptr<mir::scene::PromptSession>();
}

void qtmir::FakeSession::foreachPromptSession(std::function<void (const std::shared_ptr<mir::scene::PromptSession> &)>) const {}

void qtmir::FakeSession::setFullscreen(bool) {}

void qtmir::FakeSession::setLive(const bool) {}

void qtmir::FakeSession::appendPromptSession(const std::shared_ptr<mir::scene::PromptSession> &) {}

void qtmir::FakeSession::removePromptSession(const std::shared_ptr<mir::scene::PromptSession> &) {}

void qtmir::FakeSession::setState(qtmir::SessionInterface::State state) {
    if (m_state != state) {
        m_state = state;
        Q_EMIT stateChanged(m_state);
    }
}
