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

#include "qmirserver_p.h"

// local
#include "logging.h"
#include "mirdisplayconfigurationpolicy.h"
#include "windowmanagementpolicy.h"
#include "argvHelper.h"
#include "promptsessionmanager.h"
#include "setqtcompositor.h"

// miral
#include <miral/add_init_callback.h>
#include <miral/set_command_line_hander.h>
#include <miral/set_terminator.h>
#include <miral/set_window_managment_policy.h>

// Qt
#include <QCoreApplication>
#include <QOpenGLContext>

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

QMirServerPrivate::QMirServerPrivate(int &argc, char *argv[]) :
    runner(argc, const_cast<const char **>(argv)),
    argc{argc}, argv{argv}
{
}

PromptSessionListener *QMirServerPrivate::promptSessionListener() const
{
    return m_mirServerHooks.promptSessionListener();
}

void QMirServerPrivate::run(const std::function<void()> &startCallback)
{
    bool unknownArgsFound = false;

    miral::SetCommandLineHandler setCommandLineHandler{[this, &unknownArgsFound](int filteredCount, const char* const filteredArgv[])
    {
        unknownArgsFound = true;
        // Want to edit argv to match that which Mir returns, as those are for to Qt alone to process. Edit existing
        // argc as filteredArgv only defined in this scope.
        qtmir::editArgvToMatch(argc, argv, filteredCount, filteredArgv);
    }};

    miral::AddInitCallback addInitCallback{[&, this]
    {
        if (!unknownArgsFound) { // mir parsed all the arguments, so edit argv to pretend to have just argv[0]
            argc = 1;
        }
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
            miral::set_window_managment_policy<WindowManagementPolicy>(m_windowModelNotifier, m_windowController,
                    m_appNotifier, eventFeeder),
            qtmir::setDisplayConfigurationPolicy,
            setCommandLineHandler,
            addInitCallback,
            qtmir::SetQtCompositor{screensModel},
            setTerminator,
        });
}

void QMirServerPrivate::stop()
{
    runner.stop();
}
