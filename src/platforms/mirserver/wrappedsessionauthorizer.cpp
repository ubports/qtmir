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

#include "wrappedsessionauthorizer.h"
#include "qmirserver.h"
#include "logging.h"
#include "tracepoints.h" // generated from tracepoints.tp

#include <QMetaMethod>
#include <QThread>

namespace qtmir {

    struct SessionAuthorizer::Private
    {
        bool m_connectionChecked{false};
    };

    SessionAuthorizer::SessionAuthorizer()
        : d(new SessionAuthorizer::Private)
    {
    }

    SessionAuthorizer::~SessionAuthorizer()
    {
    }

    bool SessionAuthorizer::connectionIsAllowed(miral::ApplicationCredentials const& creds)
    {
        tracepoint(qtmirserver, sessionAuthorizeStart);
        qCDebug(QTMIR_MIR_MESSAGES) << "SessionAuthorizer::connection_is_allowed - this=" << this << "pid=" << creds.pid();
        bool authorized = true;

        if (!d->m_connectionChecked) {
            // Wait until the ApplicationManager is ready to receive requestAuthorizationForSession signals
            QMetaMethod mm = QMetaMethod::fromSignal(&SessionAuthorizer::requestAuthorizationForSession);
            for (int i = 0; i < 100 && !isSignalConnected(mm); ++i) {
                QThread::usleep(10000);
            }
            if (!isSignalConnected(mm)) {
                qCDebug(QTMIR_MIR_MESSAGES) <<
                    "SessionAuthorizer::connection_is_allowed - Gave up waiting for signal listeners";
            }
            d->m_connectionChecked = true;
        }

        Q_EMIT requestAuthorizationForSession(creds.pid(), authorized); // needs to block until authorized value returned
        tracepoint(qtmirserver, sessionAuthorizeEnd);

        return authorized;
    }

    bool SessionAuthorizer::configureDisplayIsAllowed(miral::ApplicationCredentials const& creds)
    {
        qCDebug(QTMIR_MIR_MESSAGES) << "SessionAuthorizer::configure_display_is_allowed - this=" << this << "pid=" << creds.pid();

        //FIXME(ricmm) Actually mediate this access for clients
        Q_UNUSED(creds)
        return true;
    }

    bool SessionAuthorizer::setBaseDisplayConfigurationIsAllowed(miral::ApplicationCredentials const& creds)
    {
        qCDebug(QTMIR_MIR_MESSAGES) << "SessionAuthorizer::set_base_display_configuration_is_allowed - this=" << this << "pid=" << creds.pid();

        //FIXME Actually mediate this access for clients
        Q_UNUSED(creds)
        return true;
    }

    bool SessionAuthorizer::screencastIsAllowed(miral::ApplicationCredentials const& creds)
    {
        qCDebug(QTMIR_MIR_MESSAGES) << "SessionAuthorizer::screencast_is_allowed - this=" << this << "pid=" << creds.pid();

        //FIXME Actually mediate this access for clients
        Q_UNUSED(creds)
        return true;
    }

    bool SessionAuthorizer::promptSessionIsAllowed(miral::ApplicationCredentials const& creds)
    {
        qCDebug(QTMIR_MIR_MESSAGES) << "SessionAuthorizer::prompt_session_is_allowed - this="
            << this << "pid=" << creds.pid();

        //FIXME Actually mediate this access for clients
        Q_UNUSED(creds)
        return true;
    }

    bool SessionAuthorizer::configureInputIsAllowed(miral::ApplicationCredentials const& creds)
    {
        qCDebug(QTMIR_MIR_MESSAGES) << "SessionAuthorizer::configure_input_is_allowed - this=" << this << "pid=" << creds.pid();

        //FIXME Actually mediate this access for clients
        Q_UNUSED(creds)
        return true;
    }

    bool SessionAuthorizer::setBaseInputConfigurationIsAllowed(miral::ApplicationCredentials const& creds)
    {
        qCDebug(QTMIR_MIR_MESSAGES) << "SessionAuthorizer::set_base_input_configuration_is_allowed - this=" << this << "pid=" << creds.pid();

        //FIXME Actually mediate this access for clients
        Q_UNUSED(creds)
        return true;
    }

    struct BasicSetSessionAuthorizer::Private
    {
        Private(qtmir::SessionAuthorizerBuilder const& builder) :
            builder{builder} {}

        qtmir::SessionAuthorizerBuilder builder;
    };

    BasicSetSessionAuthorizer::BasicSetSessionAuthorizer(qtmir::SessionAuthorizerBuilder const& builder)
        : d(new BasicSetSessionAuthorizer::Private(builder))
    {
    }

    void BasicSetSessionAuthorizer::operator()(QMirServer &server)
    {
        server.overrideSessionAuthorizer(d->builder);
    }

    SessionAuthorizerBuilder qtmir::BasicSetSessionAuthorizer::builder() const
    {
        return d->builder;
    }
}

WrappedSessionAuthorizer::WrappedSessionAuthorizer(const qtmir::SessionAuthorizerBuilder &builder)
    : impl(builder())
{
}

bool WrappedSessionAuthorizer::connection_is_allowed(const miral::ApplicationCredentials &creds)
{
    return impl->connectionIsAllowed(creds);
}

bool WrappedSessionAuthorizer::configure_display_is_allowed(const miral::ApplicationCredentials &creds)
{
    return impl->configureDisplayIsAllowed(creds);
}

bool WrappedSessionAuthorizer::set_base_display_configuration_is_allowed(const miral::ApplicationCredentials &creds)
{
    return impl->setBaseDisplayConfigurationIsAllowed(creds);
}

bool WrappedSessionAuthorizer::screencast_is_allowed(const miral::ApplicationCredentials &creds)
{
    return impl->screencastIsAllowed(creds);
}

bool WrappedSessionAuthorizer::prompt_session_is_allowed(const miral::ApplicationCredentials &creds)
{
    return impl->promptSessionIsAllowed(creds);
}

bool WrappedSessionAuthorizer::configure_input_is_allowed(const miral::ApplicationCredentials &creds)
{
    return impl->configureInputIsAllowed(creds);
}

bool WrappedSessionAuthorizer::set_base_input_configuration_is_allowed(const miral::ApplicationCredentials &creds)
{
    return impl->setBaseInputConfigurationIsAllowed(creds);
}