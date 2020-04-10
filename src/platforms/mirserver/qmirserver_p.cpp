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

// local
#include "logging.h"
#include "mirdisplayconfigurationpolicy.h"
#include "windowmanagementpolicy.h"
#include "promptsessionmanager.h"
#include "setqtcompositor.h"

// prototyping for later incorporation in miral
#include <miral/persist_display_config.h>

// miral
#include <miral/add_init_callback.h>
#include <miral/set_terminator.h>
#include <miral/version.h>
#include <miral/x11_support.h>
#if MIRAL_VERSION > MIR_VERSION_NUMBER(1,3,1)
#include <miral/set_window_management_policy.h>
#else
#include <miral/set_window_managment_policy.h>
#endif

// Qt
#include <QCoreApplication>
#include <QOpenGLContext>

#include <valgrind.h>

static int qtmirArgc{1};
static const char *qtmirArgv[]{"qtmir"};

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
    const int timeout = RUNNING_ON_VALGRIND ? 100 : 10; // else timeout triggers before Mir ready

    std::unique_lock<decltype(mutex)> lock(mutex);
    started_cv.wait_for(lock, std::chrono::seconds{timeout}, [&]{ return mir_running; });
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

QMirServerPrivate::QMirServerPrivate() :
    runner(qtmirArgc, qtmirArgv)
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

    runner.run_with(
        {
            m_sessionAuthorizer,
            m_openGLContextFactory,
            m_mirServerHooks,
            miral::set_window_management_policy<WindowManagementPolicy>(m_windowModelNotifier, m_windowController,
                    m_appNotifier, screensModel),
            addInitCallback,
            qtmir::SetQtCompositor{screensModel},
            setTerminator,
            miral::PersistDisplayConfig{&qtmir::wrapDisplayConfigurationPolicy},
            miral::X11Support{},
        });
}

void QMirServerPrivate::stop()
{
    runner.stop();
}
