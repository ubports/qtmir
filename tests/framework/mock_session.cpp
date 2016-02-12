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

#include "mock_session.h"

namespace qtmir
{

MockSession::MockSession()
    : SessionInterface(0)
{
    m_state = Starting;
    ON_CALL(*this, suspend()).WillByDefault(::testing::Invoke(this, &MockSession::doSuspend));
    ON_CALL(*this, resume()).WillByDefault(::testing::Invoke(this, &MockSession::doResume));
    ON_CALL(*this, stop()).WillByDefault(::testing::Invoke(this, &MockSession::doStop));
    ON_CALL(*this, state()).WillByDefault(::testing::Invoke(this, &MockSession::doState));
}

MockSession::~MockSession()
{

}

SessionInterface::State MockSession::doState() const
{
    return m_state;
}

void MockSession::doStop()
{
    setState(Stopped);
}

void MockSession::doResume()
{
    if (m_state == Suspending || m_state == Suspended) {
        setState(Running);
    }
}

void MockSession::doSuspend()
{
    if (m_state == Running) {
        setState(Suspending);
    }
}

void MockSession::setState(SessionInterface::State state)
{
    if (m_state != state) {
        m_state = state;
        Q_EMIT stateChanged(m_state);
    }
}

} // namespace qtmir
