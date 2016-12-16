/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "promptsessionmanager.h"
#include "promptsession.h"

#include <mir/scene/prompt_session_manager.h>

qtmir::PromptSessionManager::PromptSessionManager(std::shared_ptr<mir::scene::PromptSessionManager> const &promptSessionManager) :
    m_promptSessionManager{promptSessionManager}
{
}

qtmir::PromptSessionManager::~PromptSessionManager() = default;

miral::Application qtmir::PromptSessionManager::applicationFor(const PromptSession &promptSession) const
{
    return m_promptSessionManager->application_for(promptSession);
}

void qtmir::PromptSessionManager::stopPromptSession(const PromptSession &promptSession) const
{
    m_promptSessionManager->stop_prompt_session(promptSession);
}

void qtmir::PromptSessionManager::suspendPromptSession(const PromptSession &promptSession) const
{
    m_promptSessionManager->suspend_prompt_session(promptSession);
}

void qtmir::PromptSessionManager::resumePromptSession(const PromptSession &promptSession) const
{
    m_promptSessionManager->resume_prompt_session(promptSession);
}
