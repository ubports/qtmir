/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
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

#ifndef MIRSERVER_H
#define MIRSERVER_H

#include <QObject>
#include <QSharedPointer>
#include <mir/server.h>

class QtEventFeeder;
class MirDisplayConfigurationPolicy;
class SessionListener;
class SessionAuthorizer;
using MirShell = mir::shell::Shell;
class PromptSessionListener;
class ScreensModel;
class MirWindowManager;

// We use virtual inheritance of mir::Server to facilitate derived classes (e.g. testing)
// calling initialization functions before MirServer is constructed.
class MirServer : public QObject, private virtual mir::Server
{
    Q_OBJECT

    Q_PROPERTY(SessionAuthorizer* sessionAuthorizer READ sessionAuthorizer CONSTANT)
    Q_PROPERTY(SessionListener* sessionListener READ sessionListener CONSTANT)
    Q_PROPERTY(MirShell* shell READ shell CONSTANT)
    Q_PROPERTY(PromptSessionListener* promptSessionListener READ promptSessionListener CONSTANT)

public:
    MirServer(int &argc, char **argv, const QSharedPointer<ScreensModel> &, QObject* parent = 0);
    virtual ~MirServer() = default;

    /* mir specific */
    using mir::Server::run;
    using mir::Server::the_display;
    using mir::Server::the_display_configuration_controller;
    using mir::Server::the_gl_config;
    using mir::Server::the_main_loop;
    using mir::Server::the_prompt_session_manager;

    void stop();

    /* qt specific */
    // getters
    SessionAuthorizer *sessionAuthorizer();
    SessionListener *sessionListener();
    PromptSessionListener *promptSessionListener();
    MirWindowManager *windowManager();
    MirShell *shell();

private:
    std::weak_ptr<MirWindowManager> m_windowManager;
    const QSharedPointer<ScreensModel> m_screensModel;
};

#endif // MIRSERVER_H
