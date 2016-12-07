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

#ifndef QTMIR_PROMPTSESSIONMANAGER_H
#define QTMIR_PROMPTSESSIONMANAGER_H

#include <miral/application.h>

#include <memory>

namespace mir {
namespace scene {
class PromptSessionManager;
}
}

namespace qtmir {
class PromptSession;
class PromptSessionManager
{
public:
    explicit PromptSessionManager(const std::shared_ptr<mir::scene::PromptSessionManager> &promptSessionManager);
    virtual ~PromptSessionManager();

    ::miral::Application applicationFor(qtmir::PromptSession const &promptSession) const;

    void stopPromptSession(qtmir::PromptSession const &promptSession) const;
    void suspendPromptSession(qtmir::PromptSession const &promptSession) const;
    void resumePromptSession(qtmir::PromptSession const &promptSession) const;

private:
    std::shared_ptr<mir::scene::PromptSessionManager> const m_promptSessionManager;
};
}

#endif //QTMIR_PROMPTSESSIONMANAGER_H
