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

#include "mock_mir_session.h"

namespace mir
{
namespace scene
{

MockSession::MockSession()
{
}

MockSession::MockSession(const std::string &sessionName, pid_t processId)
    : m_sessionName(sessionName), m_sessionId(processId)
{
}

MockSession::~MockSession()
{
}

std::string MockSession::name() const
{
    return m_sessionName;
}

pid_t MockSession::process_id() const
{
    return m_sessionId;
}

void MockSession::resume_prompt_session() {}

void MockSession::suspend_prompt_session() {}

void MockSession::stop_prompt_session() {}

void MockSession::start_prompt_session() {}

std::shared_ptr<scene::Surface> MockSession::surface_after(const std::shared_ptr<scene::Surface> &) const
{
    return {};
}

void MockSession::configure_streams(scene::Surface &, const std::vector<mir::shell::StreamSpecification> &) {}

} // namespace scene
} // namespace mir
