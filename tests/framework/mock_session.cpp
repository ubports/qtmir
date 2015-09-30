#include "mock_session.h"

qtmir::MockSession::MockSession() : SessionInterface(0) {
    m_state = Starting;
    ON_CALL(*this, suspend()).WillByDefault(::testing::Invoke(this, &MockSession::doSuspend));
    ON_CALL(*this, resume()).WillByDefault(::testing::Invoke(this, &MockSession::doResume));
    ON_CALL(*this, stop()).WillByDefault(::testing::Invoke(this, &MockSession::doStop));
    ON_CALL(*this, state()).WillByDefault(::testing::Invoke(this, &MockSession::doState));
}

qtmir::MockSession::~MockSession()
{

}

qtmir::SessionInterface::State qtmir::MockSession::doState() const {
    return m_state;
}

void qtmir::MockSession::doStop() {
    setState(Stopped);
}

void qtmir::MockSession::doResume() {
    if (m_state == Suspending || m_state == Suspended) {
        setState(Running);
    }
}

void qtmir::MockSession::doSuspend() {
    if (m_state == Running) {
        setState(Suspending);
    }
}

void qtmir::MockSession::setState(qtmir::SessionInterface::State state) {
    if (m_state != state) {
        m_state = state;
        Q_EMIT stateChanged(m_state);
    }
}
