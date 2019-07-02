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

#ifndef WORKSPACECONTROLLER_H
#define WORKSPACECONTROLLER_H

#include "workspacecontrollerinterface.h"

class WrappedWindowManagementPolicy;

namespace qtmir {

class WorkspaceController : public qtmir::WorkspaceControllerInterface
{
public:
    WorkspaceController();
    virtual ~WorkspaceController() = default;

    void forEachWindowInWorkspace(const std::shared_ptr<miral::Workspace> &workspace,
                                  const std::function<void (const miral::Window &)> &callback) override;

    void moveWorkspaceContentToWorkspace(const std::shared_ptr<miral::Workspace> &to,
                                         const std::shared_ptr<miral::Workspace> &from) override;

    void moveWindowToWorkspace(const miral::Window &window,
                               const std::shared_ptr<miral::Workspace> &workspace) override;

    void setPolicy(WrappedWindowManagementPolicy *policy);

protected:
    WrappedWindowManagementPolicy *m_policy;
};

} // namespace qtmir

#endif // WINDOWCONTROLLER_H
