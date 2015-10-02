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
