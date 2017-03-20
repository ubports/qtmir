/*
 * Copyright (C) 2015,2017 Canonical, Ltd.
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

#include "qmirserver_p.h"
#include "qtmir/miral/display_configuration_storage.h"

// local
#include "logging.h"
#include "wrappedwindowmanagementpolicy.h"
#include "promptsessionmanager.h"
#include "setqtcompositor.h"
#include "qteventfeeder.h"
#include "qtmir/sessionauthorizer.h"

// prototyping for later incorporation in miral
#include <miral/persist_display_config.h>

// miral
#include <miral/add_init_callback.h>
#include <miral/set_terminator.h>
#include <miral/set_window_managment_policy.h>

// Qt
#include <QCoreApplication>
#include <QOpenGLContext>

namespace
{
static int qtmirArgc{1};
static const char *qtmirArgv[]{"qtmir"};

class DefaultWindowManagementPolicy : public qtmir::WindowManagementPolicy
{
public:
    DefaultWindowManagementPolicy(const miral::WindowManagerTools &tools, qtmir::WindowManagementPolicyPrivate& dd)
        : qtmir::WindowManagementPolicy(tools, dd)
    {}
};

struct DefaultDisplayConfigurationStorage : miral::DisplayConfigurationStorage
{
    void save(const miral::DisplayId&, const miral::DisplayConfigurationOptions&) override {}

    bool load(const miral::DisplayId&, miral::DisplayConfigurationOptions&) const override { return false; }
};

auto buildDisplayConfigurationPolicy()
-> std::shared_ptr<miral::DisplayConfigurationPolicy>
{
    return std::make_shared<qtmir::DisplayConfigurationPolicy>();
}

auto buildDisplayConfigurationStorage()
-> std::shared_ptr<miral::DisplayConfigurationStorage>
{
    return std::make_shared<DefaultDisplayConfigurationStorage>();
}

auto buildWindowManagementPolicy(const miral::WindowManagerTools &tools, qtmir::WindowManagementPolicyPrivate& dd)
-> std::shared_ptr<qtmir::WindowManagementPolicy>
{
    return std::make_shared<DefaultWindowManagementPolicy>(tools, dd);
}

auto buildSessionAuthorizer()
-> std::shared_ptr<qtmir::SessionAuthorizer>
{
    return std::make_shared<qtmir::SessionAuthorizer>();
}

}

void MirServerThread::run()
{
    auto start_callback = [this]
    {
        std::lock_guard<std::mutex> lock(mutex);
        mir_running = true;
        started_cv.notify_one();
    };

    server->run(start_callback);

    Q_EMIT stopped();
}

bool MirServerThread::waitForMirStartup()
{
    std::unique_lock<decltype(mutex)> lock(mutex);
    started_cv.wait_for(lock, std::chrono::seconds{10}, [&]{ return mir_running; });
    return mir_running;
}

QPlatformOpenGLContext *QMirServerPrivate::createPlatformOpenGLContext(QOpenGLContext *context) const
{
    return m_openGLContextFactory.createPlatformOpenGLContext(context->format(), *m_mirServerHooks.theMirDisplay());
}

std::shared_ptr<qtmir::PromptSessionManager> QMirServerPrivate::promptSessionManager() const
{
    return std::make_shared<qtmir::PromptSessionManager>(m_mirServerHooks.thePromptSessionManager());
}

std::shared_ptr<qtmir::SessionAuthorizer> QMirServerPrivate::theApplicationAuthorizer() const
{
    auto wrapped = m_wrappedSessionAuthorizer.the_custom_application_authorizer();
    return wrapped ? wrapped->wrapper() : nullptr;
}

QMirServerPrivate::QMirServerPrivate()
    : m_displayConfigurationPolicy(buildDisplayConfigurationPolicy)
    , m_windowManagementPolicy(buildWindowManagementPolicy)
    , m_displayConfigurationStorage(buildDisplayConfigurationStorage)
    , m_wrappedSessionAuthorizer(buildSessionAuthorizer)
    , runner(qtmirArgc, qtmirArgv)
{
}

PromptSessionListener *QMirServerPrivate::promptSessionListener() const
{
    return m_mirServerHooks.promptSessionListener();
}

void QMirServerPrivate::run(const std::function<void()> &startCallback)
{

    miral::AddInitCallback addInitCallback{[&, this]
    {
        qCDebug(QTMIR_MIR_MESSAGES) << "MirServer created";
        qCDebug(QTMIR_MIR_MESSAGES) << "Command line arguments passed to Qt:" << QCoreApplication::arguments();
    }};

    miral::SetTerminator setTerminator{[](int)
    {
        qDebug() << "Signal caught by Mir, stopping Mir server..";
        QCoreApplication::quit();
    }};

    runner.set_exception_handler([this]
    {
        try {
            throw;
        } catch (const std::exception &ex) {
            qCritical() << ex.what();
            exit(1);
        }
    });

    runner.add_start_callback([&]
    {
        screensModel->update();
        screensController = m_mirServerHooks.createScreensController(screensModel);
        m_mirServerHooks.createInputDeviceObserver();
    });

    runner.add_start_callback(startCallback);

    runner.add_stop_callback([&]
    {
        screensModel->terminate();
        screensController.clear();
    });

    auto eventFeeder = std::make_shared<QtEventFeeder>(screensModel);
    auto displayStorageBuilder = m_displayConfigurationStorage.builder();

    runner.run_with(
        {
            m_wrappedSessionAuthorizer,
            m_openGLContextFactory,
            m_mirServerHooks,
            miral::set_window_management_policy<WrappedWindowManagementPolicy>(m_windowModelNotifier,
                                                                               m_windowController,
                                                                               m_appNotifier,
                                                                               eventFeeder,
                                                                               m_windowManagementPolicy),
            addInitCallback,
            qtmir::SetQtCompositor{screensModel},
            setTerminator,
            miral::PersistDisplayConfig{displayStorageBuilder(),
                                        m_displayConfigurationPolicy}
        });
}

void QMirServerPrivate::stop()
{
    runner.stop();
}
