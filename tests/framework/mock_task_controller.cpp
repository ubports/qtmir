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

#include "mock_task_controller.h"

namespace qtmir
{

MockTaskController::MockTaskController()
{
    using namespace ::testing;
    ON_CALL(*this, primaryPidForAppId(_))
            .WillByDefault(
                Invoke(this, &MockTaskController::doPrimaryPidForAppId));

    ON_CALL(*this, appIdHasProcessId(_, _))
            .WillByDefault(
                Invoke(this, &MockTaskController::doAppIdHasProcessId));

    ON_CALL(*this, findDesktopFileForAppId(_))
            .WillByDefault(
                Invoke(this, &MockTaskController::doFindDesktopFileForAppId));

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

pid_t MockTaskController::doPrimaryPidForAppId(const QString &appId)
{
    auto it = children.find(appId);
    if (it == children.end())
        return -1;

    return it->pid();
}


bool MockTaskController::doAppIdHasProcessId(const QString &appId, pid_t pid)
{
    auto primaryPid = primaryPidForAppId(appId);
    if (primaryPid == -1)
        return false;

    return primaryPid == pid;
}


QFileInfo MockTaskController::doFindDesktopFileForAppId(const QString &appId) const
{
    QString path = QString("/usr/share/applications/%1.desktop").arg(appId);
    return QFileInfo(path);
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
