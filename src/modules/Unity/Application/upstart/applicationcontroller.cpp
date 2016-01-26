/*
 * Copyright (C) 2014,2015 Canonical, Ltd.
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

#include "applicationcontroller.h"
#include "applicationinfo.h"

// upstart
extern "C" {
    #include "ubuntu-app-launch.h"
}
#include <ubuntu-app-launch/registry.h>

namespace ual = Ubuntu::AppLaunch;

namespace qtmir
{
namespace upstart
{

struct ApplicationController::Private
{
    std::shared_ptr<ual::Registry> registry;
    UbuntuAppLaunchAppObserver preStartCallback = nullptr;
    UbuntuAppLaunchAppObserver startedCallback = nullptr;
    UbuntuAppLaunchAppObserver stopCallback = nullptr;
    UbuntuAppLaunchAppObserver focusCallback = nullptr;
    UbuntuAppLaunchAppObserver resumeCallback = nullptr;
    UbuntuAppLaunchAppPausedResumedObserver pausedCallback = nullptr;
    UbuntuAppLaunchAppFailedObserver failureCallback = nullptr;
};

namespace {
/**
 * @brief toShortAppIdIfPossible
 * @param appId - any string that you think is an appId
 * @return if a valid appId was input, a shortened appId is returned, else returns the input string unaltered
 */
QString toShortAppIdIfPossible(const QString &appId) {
    gchar *package, *application;
    if (ubuntu_app_launch_app_id_parse(appId.toLatin1().constData(), &package, &application, nullptr)) {
        // is long appId, so assemble its short appId
        QString shortAppId = QString("%1_%2").arg(package).arg(application);
        g_free(package);
        g_free(application);
        return shortAppId;
    } else {
        return appId;
    }
}

std::shared_ptr<ual::Application> createApp(const QString &inputAppId, std::shared_ptr<ual::Registry> registry)
{
    QStringList pieces = inputAppId.split("_");
    ual::AppID appId;
    switch (pieces.size()) {
    case 2:
        appId = ual::AppID::discover(pieces[0].toStdString(), pieces[1].toStdString());
        break;
    default:
        appId = ual::AppID::parse(inputAppId.toStdString());
        break;
    }
    return ual::Application::create(appId, registry);
}

} // namespace

ApplicationController::ApplicationController()
    : qtmir::ApplicationController(),
      impl(new Private())
{
    impl->registry = std::make_shared<ual::Registry>();

    impl->preStartCallback = [](const gchar * appId, gpointer userData) {
        auto thiz = static_cast<ApplicationController*>(userData);
        Q_EMIT(thiz->applicationAboutToBeStarted(toShortAppIdIfPossible(appId)));
    };

    impl->startedCallback = [](const gchar * appId, gpointer userData) {
        auto thiz = static_cast<ApplicationController*>(userData);
        Q_EMIT(thiz->applicationStarted(toShortAppIdIfPossible(appId)));
    };

    impl->stopCallback = [](const gchar * appId, gpointer userData) {
        auto thiz = static_cast<ApplicationController*>(userData);
        Q_EMIT(thiz->applicationStopped(toShortAppIdIfPossible(appId)));
    };

    impl->focusCallback = [](const gchar * appId, gpointer userData) {
        auto thiz = static_cast<ApplicationController*>(userData);
        Q_EMIT(thiz->applicationFocusRequest(toShortAppIdIfPossible(appId)));
    };

    impl->resumeCallback = [](const gchar * appId, gpointer userData) {
        auto thiz = static_cast<ApplicationController*>(userData);
        Q_EMIT(thiz->applicationResumeRequested(toShortAppIdIfPossible(appId)));
    };

    impl->pausedCallback = [](const gchar * appId, GPid *, gpointer userData) {
        auto thiz = static_cast<ApplicationController*>(userData);
        Q_EMIT(thiz->applicationPaused(toShortAppIdIfPossible(appId)));
    };

    impl->failureCallback = [](const gchar * appId, UbuntuAppLaunchAppFailed failureType, gpointer userData) {
        ApplicationController::Error error;
        switch(failureType)
        {
        case UBUNTU_APP_LAUNCH_APP_FAILED_CRASH: error = ApplicationController::Error::APPLICATION_CRASHED;
        case UBUNTU_APP_LAUNCH_APP_FAILED_START_FAILURE: error = ApplicationController::Error::APPLICATION_FAILED_TO_START;
        }

        auto thiz = static_cast<ApplicationController*>(userData);
        Q_EMIT(thiz->applicationError(toShortAppIdIfPossible(appId), error));
    };

    ubuntu_app_launch_observer_add_app_starting(impl->preStartCallback, this);
    ubuntu_app_launch_observer_add_app_started(impl->startedCallback, this);
    ubuntu_app_launch_observer_add_app_stop(impl->stopCallback, this);
    ubuntu_app_launch_observer_add_app_focus(impl->focusCallback, this);
    ubuntu_app_launch_observer_add_app_resume(impl->resumeCallback, this);
    ubuntu_app_launch_observer_add_app_paused(impl->pausedCallback, this);
    ubuntu_app_launch_observer_add_app_failed(impl->failureCallback, this);
}

ApplicationController::~ApplicationController()
{
    ubuntu_app_launch_observer_delete_app_starting(impl->preStartCallback, this);
    ubuntu_app_launch_observer_delete_app_started(impl->startedCallback, this);
    ubuntu_app_launch_observer_delete_app_stop(impl->stopCallback, this);
    ubuntu_app_launch_observer_delete_app_focus(impl->focusCallback, this);
    ubuntu_app_launch_observer_delete_app_resume(impl->resumeCallback, this);
    ubuntu_app_launch_observer_delete_app_paused(impl->pausedCallback, this);
    ubuntu_app_launch_observer_delete_app_failed(impl->failureCallback, this);
}

bool ApplicationController::appIdHasProcessId(pid_t pid, const QString& appId)
{
    auto app = createApp(appId, impl->registry);

    for (auto &instance: app->instances()) {
        if (instance->hasPid(pid)) {
            return true;
        }
    }

    return false;
}

bool ApplicationController::stopApplicationWithAppId(const QString& appId)
{
    auto app = createApp(appId, impl->registry);

    for (auto &instance: app->instances()) {
        instance->stop();
    }

    return true;
}

bool ApplicationController::startApplicationWithAppIdAndArgs(const QString& appId, const QStringList& arguments)
{
    auto app = createApp(appId, impl->registry);

    // Convert arguments QStringList into format suitable for ubuntu-app-launch
    std::vector<ual::Application::URL> urls;
    for (auto &arg: arguments) {
        urls.emplace_back(ual::Application::URL::from_raw(arg.toStdString()));
    }

    app->launch(urls);

    return true;
}

bool ApplicationController::pauseApplicationWithAppId(const QString& appId)
{
    auto app = createApp(appId, impl->registry);

    for (auto &instance: app->instances()) {
        instance->pause();
    }

    return true;
}

bool ApplicationController::resumeApplicationWithAppId(const QString& appId)
{
    auto app = createApp(appId, impl->registry);

    for (auto &instance: app->instances()) {
        instance->resume();
    }

    return true;
}

std::shared_ptr<qtmir::ApplicationInfo> ApplicationController::getInfoForApp(const QString &appId) const
{
    auto app = createApp(appId, impl->registry);
    QString shortAppId = toShortAppIdIfPossible(QString::fromStdString(std::string(app->appId())));
    return std::make_shared<qtmir::upstart::ApplicationInfo>(shortAppId, app->info());
}

} // namespace upstart
} // namespace qtmir
