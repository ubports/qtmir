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

#ifndef MOCK_APPLICATION_CONTROLLER_H
#define MOCK_APPLICATION_CONTROLLER_H

#include <Unity/Application/applicationcontroller.h>
#include <QMap>

#include <core/posix/fork.h>

#include <gmock/gmock.h>

namespace testing
{
struct MockApplicationController : public qtmir::ApplicationController
{
    MockApplicationController();
    virtual ~MockApplicationController();

    MOCK_METHOD1(primaryPidForAppId, pid_t(const QString& appId));
    MOCK_METHOD2(appIdHasProcessId, bool(pid_t, const QString&));
    MOCK_CONST_METHOD1(findDesktopFileForAppId, QFileInfo(const QString &appId));

    MOCK_METHOD1(stopApplicationWithAppId, bool(const QString&));
    MOCK_METHOD2(startApplicationWithAppIdAndArgs, bool(const QString&, const QStringList&));
    MOCK_METHOD1(pauseApplicationWithAppId, bool(const QString&));
    MOCK_METHOD1(resumeApplicationWithAppId, bool(const QString&));

    pid_t doPrimaryPidForAppId(const QString& appId);

    bool doAppIdHasProcessId(pid_t pid, const QString& appId);

    QFileInfo doFindDesktopFileForAppId(const QString& appId) const;

    bool doStopApplicationWithAppId(const QString& appId);

    bool doStartApplicationWithAppIdAndArgs(const QString& appId, const QStringList& args);

    bool doPauseApplicationWithAppId(const QString& appId);

    bool doResumeApplicationWithAppId(const QString& appId);

private:
    QMap<QString, core::posix::ChildProcess> children;
};
} // namespace testing

#endif // MOCK_APPLICATION_CONTROLLER_H
