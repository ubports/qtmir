/*
 * Copyright (C) 2016 Canonical, Ltd.
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

#ifndef QTMIR_SESSIONAUTHORIZER_H
#define QTMIR_SESSIONAUTHORIZER_H

// Qt
#include <QObject>

// mir
#include <miral/application_authorizer.h>

class QMirServer;

namespace qtmir {

/*
    Provides callbacks to grant/deny access for session capabilities
 */
class SessionAuthorizer : public QObject
{
    Q_OBJECT
public:
    SessionAuthorizer();
    ~SessionAuthorizer();

    virtual bool connectionIsAllowed(miral::ApplicationCredentials const& creds);
    virtual bool configureDisplayIsAllowed(miral::ApplicationCredentials const& creds);
    virtual bool setBaseDisplayConfigurationIsAllowed(miral::ApplicationCredentials const& creds);
    virtual bool screencastIsAllowed(miral::ApplicationCredentials const& creds);
    virtual bool promptSessionIsAllowed(miral::ApplicationCredentials const& creds);
    virtual bool configureInputIsAllowed(miral::ApplicationCredentials const& creds);
    virtual bool setBaseInputConfigurationIsAllowed(miral::ApplicationCredentials const& creds);
Q_SIGNALS:
    // needs to be blocked queued signal which returns value for authorized
    void requestAuthorizationForSession(const pid_t &pid, bool &authorized);

private:
    struct Private;
    std::shared_ptr<Private> d;
};

using SessionAuthorizerBuilder =
    std::function<std::shared_ptr<SessionAuthorizer>()>;

class BasicSetSessionAuthorizer
{
public:
    explicit BasicSetSessionAuthorizer(SessionAuthorizerBuilder const& builder);
    ~BasicSetSessionAuthorizer() = default;

    void operator()(QMirServer& server);
    SessionAuthorizerBuilder builder() const;

private:
    struct Private;
    std::shared_ptr<Private> d;
};

/*
    Set the session authorizer to allow server customization

    usage:
    class MySessionAuthorizer : publi qtmir::SessionAuthorizer
    {
    ...
    }

    qtmir::GuiServerApplication app(argc, argv, { SetSessionAuthorizer<MySessionAuthorizer>() });
 */
template<typename Policy>
class SetSessionAuthorizer : public BasicSetSessionAuthorizer
{
public:
    template<typename ...Args>
    explicit SetSessionAuthorizer(Args const& ...args) :
        BasicSetSessionAuthorizer{[&args...]() { return std::make_shared<Policy>(args...); }} {}
};

} // namespace qtmir

#endif // QTMIR_SESSIONAUTHORIZER_H
