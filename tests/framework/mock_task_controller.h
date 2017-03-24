/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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

#ifndef MOCK_TASK_CONTROLLER_H
#define MOCK_TASK_CONTROLLER_H

#include <Unity/Application/taskcontroller.h>
#include <QMap>

#include <core/posix/fork.h>

#include <gmock/gmock.h>

namespace qtmir
{
struct MockTaskController : public qtmir::TaskController
{
    MockTaskController(std::shared_ptr<qtmir::PromptSessionManager> promptSessionManager, QObject *parent = nullptr);
    virtual ~MockTaskController();

    MOCK_METHOD2(appIdHasProcessId, bool(const QString&, pid_t));
    MOCK_CONST_METHOD1(getInfoForApp, QSharedPointer<qtmir::ApplicationInfo> (const QString &));

    MOCK_METHOD1(stop, bool(const QString&));
    MOCK_METHOD2(start, bool(const QString&, const QStringList&));
    MOCK_METHOD1(suspend, bool(const QString&));
    MOCK_METHOD1(resume, bool(const QString&));

    bool doAppIdHasProcessId(const QString& appId, pid_t pid);

    QSharedPointer<qtmir::ApplicationInfo> doGetInfoForApp(const QString& appId) const;

    bool doStop(const QString& appId);

    bool doStart(const QString& appId, const QStringList& args);

    bool doSuspend(const QString& appId);

    bool doResume(const QString& appId);

private:
    QMap<QString, core::posix::ChildProcess> children;
};

} // namespace qtmir

#endif // MOCK_TASK_CONTROLLER_H
