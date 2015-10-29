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

#include <QCoreApplication>

#include "mirserver.h"

// local
#include "mirwindowmanager.h"
#include "mirglconfig.h"
#include "mirserverstatuslistener.h"
#include "promptsessionlistener.h"
#include "screencontroller.h"
#include "sessionlistener.h"
#include "sessionauthorizer.h"
#include "qtcompositor.h"
#include "qteventfeeder.h"
#include "tileddisplayconfigurationpolicy.h"
#include "logging.h"

// std
#include <memory>

// egl
#define MESA_EGL_NO_X11_HEADERS
#include <EGL/egl.h>

// mir
#include <mir/graphics/cursor.h>

namespace mg = mir::graphics;
namespace mo  = mir::options;
namespace msh = mir::shell;
namespace ms = mir::scene;

Q_LOGGING_CATEGORY(QTMIR_MIR_MESSAGES, "qtmir.mir")

MirServer::MirServer(int &argc, char **argv,
                     const QSharedPointer<ScreenController> &screenController, QObject* parent)
    : QObject(parent)
    , m_screenController(screenController)
{
    bool unknownArgsFound = false;
    set_command_line_handler([&argc, &argv, &unknownArgsFound](int argc2, char const* const argv2[]) {
        unknownArgsFound = true;
        // argv2 - Mir parses out arguments that it understands. It also removes argv[0], but we will put it back.
        argc = argc2+1;
        for (int i=1; i<argc; i++) {
            argv[i] = const_cast<char*>(argv2[i-1]);
        }
        argv[argc] = nullptr;

        if (QTMIR_MIR_MESSAGES().isDebugEnabled()) {
            qDebug() << "Command line arguments passed to Qt:";
            for (int i=0; i<argc; i++) {
                qDebug() << i << argv[i];
            }
        }
    });

    // Casting char** to be a const char** only safe as Mir
    set_command_line(argc, const_cast<const char **>(argv));

    override_the_session_listener([]
        {
            return std::make_shared<SessionListener>();
        });

    override_the_prompt_session_listener([]
        {
            return std::make_shared<PromptSessionListener>();
        });

    override_the_session_authorizer([]
        {
            return std::make_shared<SessionAuthorizer>();
        });

    override_the_compositor([]
        {
            return std::make_shared<QtCompositor>();
        });

    override_the_input_dispatcher([&screenController]
        {
            return std::make_shared<QtEventFeeder>(screenController);
        });

    override_the_gl_config([]
        {
            return std::make_shared<MirGLConfig>();
        });

    override_the_server_status_listener([]
        {
            return std::make_shared<MirServerStatusListener>();
        });

    override_the_window_manager_builder([this](mir::shell::FocusController* focus_controller)
        -> std::shared_ptr<mir::shell::WindowManager>
        {
            return {MirWindowManager::create(focus_controller, the_shell_display_layout())};
        });

    wrap_display_configuration_policy(
        [](const std::shared_ptr<mg::DisplayConfigurationPolicy> &wrapped)
            -> std::shared_ptr<mg::DisplayConfigurationPolicy>
        {
            return std::make_shared<TiledDisplayConfigurationPolicy>(wrapped);
        });

    set_terminator([](int)
        {
            qDebug() << "Signal caught by Mir, stopping Mir server..";
            QCoreApplication::quit();
        });

    add_init_callback([this, &screenController] {
        screenController->init(the_display(), the_compositor());
    });

    try {
        apply_settings();
    } catch (const std::exception &ex) {
        qCritical() << ex.what();
        exit(1);
    }

    if (!unknownArgsFound) { // mir parsed all the arguments, so manually construct argv ourselves
        argc = 1;
    }

    // We will draw our own cursor.
    // FIXME: Call override_the_cusor() instead once this method becomes available in a
    //        future version of Mir.
    add_init_callback([this]() {
        the_cursor()->hide();
        // Hack to work around https://bugs.launchpad.net/mir/+bug/1502200
        static_cast<QtCompositor*>(the_compositor().get())->setCursor(the_cursor());
    });

    qCDebug(QTMIR_MIR_MESSAGES) << "MirServer created";
}

// Override default implementation to ensure we terminate the ScreenController first.
// Code path followed when Qt tries to shutdown the server.
void MirServer::stop()
{
    m_screenController->terminate();
    mir::Server::stop();
}


/************************************ Shell side ************************************/

//
// Note about the
//     if (sharedPtr.unique()) return 0;
// constructs used in the functions below.
// The rationale is that if when you do
//     the_session_authorizer()
// get a pointer that is unique means that Mir is not
// holding the pointer and thus when we return from the
//     sessionAuthorizer()
// scope the unique pointer will be destroyed so we return 0
//

SessionAuthorizer *MirServer::sessionAuthorizer()
{
    auto sharedPtr = the_session_authorizer();
    if (sharedPtr.unique()) return 0;

    return static_cast<SessionAuthorizer*>(sharedPtr.get());
}

SessionListener *MirServer::sessionListener()
{
    auto sharedPtr = the_session_listener();
    if (sharedPtr.unique()) return 0;

    return static_cast<SessionListener*>(sharedPtr.get());
}

PromptSessionListener *MirServer::promptSessionListener()
{
    auto sharedPtr = the_prompt_session_listener();
    if (sharedPtr.unique()) return 0;

    return static_cast<PromptSessionListener*>(sharedPtr.get());
}

MirShell *MirServer::shell()
{
    std::weak_ptr<MirShell> m_shell = the_shell();
    return m_shell.lock().get();
}
