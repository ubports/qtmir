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

class SessionAuthorizer : public QObject, public miral::ApplicationAuthorizer
{
    Q_OBJECT
public:
    SessionAuthorizer(QObject *parent = 0);

    bool connection_is_allowed(miral::ApplicationCredentials const& creds) override;
    bool configure_display_is_allowed(miral::ApplicationCredentials const& creds) override;
    bool set_base_display_configuration_is_allowed(miral::ApplicationCredentials const& creds) override;
    bool screencast_is_allowed(miral::ApplicationCredentials const& creds) override;
    bool prompt_session_is_allowed(miral::ApplicationCredentials const& creds) override;

Q_SIGNALS:
    // needs to be blocked queued signal which returns value for authorized
    void requestAuthorizationForSession(const pid_t &pid, bool &authorized);

private:
    struct Private;
    std::shared_ptr<Private> d;
};

class BasicSetSessionAuthorizer
{
public:
    explicit BasicSetSessionAuthorizer(miral::BasicSetApplicationAuthorizer const& builder);
    ~BasicSetSessionAuthorizer() = default;

    void operator()(QMirServer& server);

private:
    struct Private;
    std::shared_ptr<Private> d;
};

template<typename Policy>
class SetSessionAuthorizer : public BasicSetSessionAuthorizer
{
public:
    template<typename ...Args>
    explicit SetSessionAuthorizer(Args const& ...args) :
            BasicSetSessionAuthorizer{miral::SetApplicationAuthorizer<Policy>(args...)} {}
};

} // namespace qtmir

#endif // QTMIR_SESSIONAUTHORIZER_H
