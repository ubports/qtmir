/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
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
 *
 */

#ifndef APPLICATION_CONTROLLER_H
#define APPLICATION_CONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>

namespace qtmir
{

class ApplicationInfo;

class ApplicationController : public QObject
{
    Q_OBJECT

public:
    enum class Error
    {
        APPLICATION_CRASHED,
        APPLICATION_FAILED_TO_START
    };

    ApplicationController(const ApplicationController&) = delete;
    virtual ~ApplicationController() = default;

    ApplicationController& operator=(const ApplicationController&) = delete;

    virtual bool appIdHasProcessId(pid_t pid, const QString &appId) = 0;

    virtual bool stopApplicationWithAppId(const QString &appId) = 0;
    virtual bool startApplicationWithAppIdAndArgs(const QString &appId, const QStringList &arguments) = 0;

    virtual bool pauseApplicationWithAppId(const QString &appId) = 0;
    virtual bool resumeApplicationWithAppId(const QString &appId) = 0;

    virtual qtmir::ApplicationInfo *getInfoForApp(const QString &appId) const = 0;

Q_SIGNALS:
    void applicationAboutToBeStarted(const QString &appId);
    void applicationStarted(const QString &appId);
    void applicationStopped(const QString &appId);
    void applicationPaused(const QString &appId);
    void applicationFocusRequest(const QString &appId);
    void applicationResumeRequested(const QString &appId);

    void applicationError(const QString &appId, ApplicationController::Error error);

protected:
    ApplicationController() = default;
};

} // namespace qtmir

#endif // APPLICATION_CONTROLLER_H
