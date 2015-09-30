#include "mock_mir_session.h"

mir::scene::MockSession::MockSession()
{
}

mir::scene::MockSession::MockSession(const std::string &sessionName, pid_t processId)
    : m_sessionName(sessionName), m_sessionId(processId)
{
}

mir::scene::MockSession::~MockSession()
{
}

std::string mir::scene::MockSession::name() const
{
    return m_sessionName;
}

pid_t mir::scene::MockSession::process_id() const
{
    return m_sessionId;
}

void mir::scene::MockSession::resume_prompt_session() {}

void mir::scene::MockSession::suspend_prompt_session() {}

void mir::scene::MockSession::stop_prompt_session() {}

void mir::scene::MockSession::start_prompt_session() {}

std::shared_ptr<mir::scene::Surface> mir::scene::MockSession::surface_after(const std::shared_ptr<mir::scene::Surface> &) const
{
    return {};
}

void mir::scene::MockSession::configure_streams(mir::scene::Surface &, const std::vector<mir::shell::StreamSpecification> &) {}
