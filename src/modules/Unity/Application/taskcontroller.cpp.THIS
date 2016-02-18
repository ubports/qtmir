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

// local
#include "taskcontroller.h"
#include "logging.h"

// Qt
#include <QStringList>

// STL
#include <mutex>

// std
#include <csignal>
#include <unistd.h>

namespace qtmir
{

TaskController::TaskController(
        QObject *parent,
        const QSharedPointer<ApplicationController> &appController) :
    QObject(parent),
    m_appController(appController)
{
    connect(m_appController.data(),
            &ApplicationController::applicationAboutToBeStarted,
            this,
            &TaskController::processStarting);

    connect(m_appController.data(),
            &ApplicationController::applicationStopped,
            this,
            &TaskController::processStopped);

    connect(m_appController.data(),
            &ApplicationController::applicationPaused,
            this,
            &TaskController::processSuspended);

    connect(m_appController.data(),
            &ApplicationController::applicationFocusRequest,
            this,
            &TaskController::focusRequested);

    connect(m_appController.data(),
            &ApplicationController::applicationResumeRequested,
            this,
            &TaskController::resumeRequested);

    connect(m_appController.data(),
            &ApplicationController::applicationError,
            this,
            &TaskController::onApplicationError);
}

TaskController::~TaskController()
{
}

bool TaskController::start(const QString& appId, const QStringList& arguments)
{
    qCDebug(QTMIR_APPLICATIONS) << "TaskController::start - appId=" << appId;
    return m_appController->startApplicationWithAppIdAndArgs(appId, arguments);
}

bool TaskController::stop(const QString& appId)
{
    qCDebug(QTMIR_APPLICATIONS) << "TaskController::stop - appId=" << appId;
    auto result = m_appController->stopApplicationWithAppId(appId);
    if (!result)
        qCDebug(QTMIR_APPLICATIONS) << "TaskController::stopApplication - FAILED to stop appId=" << appId;

    return result;
}

bool TaskController::appIdHasProcessId(const QString& appId, const pid_t pid) const
{
    qCDebug(QTMIR_APPLICATIONS) << "TaskController::isApplicationPid - appId=" << appId << "pid=" << pid;
    return m_appController->appIdHasProcessId(pid, appId);
}

qtmir::ApplicationInfo *TaskController::getInfoForApp(const QString &appId) const
{
    return m_appController->getInfoForApp(appId);
}

bool TaskController::suspend(const QString &appId)
{
    qCDebug(QTMIR_APPLICATIONS) << "TaskController::suspend - appId=" << appId;
    return m_appController->pauseApplicationWithAppId(appId);
}

bool TaskController::resume(const QString &appId)
{
    qCDebug(QTMIR_APPLICATIONS) << "TaskController::resume - appId=" << appId;
    return m_appController->resumeApplicationWithAppId(appId);
}

void TaskController::onApplicationError(const QString& id, ApplicationController::Error error)
{
    Q_EMIT processFailed(id, (error == ApplicationController::Error::APPLICATION_FAILED_TO_START) );
}

} // namespace qtmir
