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

#ifndef WRAPPEDSESSIONAUTHORIZER_H
#define WRAPPEDSESSIONAUTHORIZER_H

#include "qtmir/sessionauthorizer.h"

class WrappedSessionAuthorizer : public miral::ApplicationAuthorizer
{
public:
    WrappedSessionAuthorizer(qtmir::SessionAuthorizerBuilder const& builder);

    virtual bool connection_is_allowed(miral::ApplicationCredentials const& creds) override;
    virtual bool configure_display_is_allowed(miral::ApplicationCredentials const& creds) override;
    virtual bool set_base_display_configuration_is_allowed(miral::ApplicationCredentials const& creds) override;
    virtual bool screencast_is_allowed(miral::ApplicationCredentials const& creds) override;
    virtual bool prompt_session_is_allowed(miral::ApplicationCredentials const& creds) override;
    virtual bool configure_input_is_allowed(miral::ApplicationCredentials const& creds) override;
    virtual bool set_base_input_configuration_is_allowed(miral::ApplicationCredentials const& creds) override;

    std::shared_ptr<qtmir::SessionAuthorizer> wrapper() { return impl; }

private:
    std::shared_ptr<qtmir::SessionAuthorizer> impl;
};

#endif // WRAPPEDSESSIONAUTHORIZER_H
