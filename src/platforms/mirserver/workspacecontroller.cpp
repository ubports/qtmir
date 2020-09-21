/*
 * Copyright (C) 2017 Canonical, Ltd.
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

#include "workspacecontroller.h"
#include "wrappedwindowmanagementpolicy.h"

using namespace qtmir;


WorkspaceController::WorkspaceController()
    : m_policy(nullptr)
{
}

void WorkspaceController::forEachWindowInWorkspace(const std::shared_ptr<miral::Workspace> &workspace, const std::function<void (const miral::Window &)> &callback)
{
    if (m_policy) {
        m_policy->for_each_window_in_workspace(workspace, callback);
    }
}

void WorkspaceController::moveWorkspaceContentToWorkspace(const std::shared_ptr<miral::Workspace> &to,
                                                          const std::shared_ptr<miral::Workspace> &from)
{
    if (m_policy) {
        m_policy->move_worspace_content_to_workspace(to, from);
    }
}

void WorkspaceController::moveWindowToWorkspace(const miral::Window &window,
                                                const std::shared_ptr<miral::Workspace> &workspace)
{
    if (m_policy) {
        m_policy->move_window_to_workspace(window, workspace);
    }
}

void WorkspaceController::setPolicy(WrappedWindowManagementPolicy * const policy)
{
    m_policy = policy;
}
