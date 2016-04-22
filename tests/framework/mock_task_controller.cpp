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

#include "mock_application_info.h"
#include "mock_task_controller.h"

namespace qtmir
{

MockTaskController::MockTaskController()
{
    using namespace ::testing;

    ON_CALL(*this, appIdHasProcessId(_, _))
            .WillByDefault(
                Invoke(this, &MockTaskController::doAppIdHasProcessId));

    ON_CALL(*this, getInfoForApp(_))
            .WillByDefault(
                Invoke(this, &MockTaskController::doGetInfoForApp));

    ON_CALL(*this, stop(_))
            .WillByDefault(
                Invoke(this, &MockTaskController::doStop));

    ON_CALL(*this, start(_, _))
            .WillByDefault(
                Invoke(this, &MockTaskController::doStart));

    ON_CALL(*this, suspend(_))
            .WillByDefault(
                Invoke(this, &MockTaskController::doSuspend));

    ON_CALL(*this, resume(_))
            .WillByDefault(
                Invoke(this, &MockTaskController::doResume));
}

MockTaskController::~MockTaskController()
{

}


bool MockTaskController::doAppIdHasProcessId(const QString &appId, pid_t pid)
{
    auto it = children.find(appId);
    if (it == children.end())
        return false;

    return it->pid() == pid;
}


QSharedPointer<qtmir::ApplicationInfo> MockTaskController::doGetInfoForApp(const QString& appId) const
{
    return QSharedPointer<qtmir::ApplicationInfo>(new testing::NiceMock<MockApplicationInfo>(appId));
}


bool MockTaskController::doStop(const QString &appId)
{
    Q_UNUSED(appId);

    return false;
}


bool MockTaskController::doStart(const QString &appId, const QStringList &args)
{
    Q_UNUSED(args);

    auto child = core::posix::fork([]()
    {
        while (true);

        return core::posix::exit::Status::success;
    }, core::posix::StandardStream::empty);

    if (child.pid() > 0)
    {
        children.insert(appId, child);
        return true;
    }

    return false;
}


bool MockTaskController::doSuspend(const QString &appId)
{
    Q_UNUSED(appId);

    return false;
}

bool MockTaskController::doResume(const QString &appId)
{
    Q_UNUSED(appId);

    return false;
}

} // namespace qtmir
