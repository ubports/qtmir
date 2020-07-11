/*
 * Copyright (C) 2017 Canonical, Ltd.
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

#ifndef MIRDISPLAYCONFIGURATIONOBSERVER_H
#define MIRDISPLAYCONFIGURATIONOBSERVER_H

#include <QObject>

#include <mir/graphics/display_configuration_observer.h>
#include <mir/version.h>

class MirDisplayConfigurationObserver : public QObject,
                                        public mir::graphics::DisplayConfigurationObserver
{
    Q_OBJECT
public:
    explicit MirDisplayConfigurationObserver(QObject *parent = 0);

    void initial_configuration(std::shared_ptr<mir::graphics::DisplayConfiguration const> const& config) override;

    void configuration_applied(std::shared_ptr<mir::graphics::DisplayConfiguration const> const& config) override;

    void base_configuration_updated(std::shared_ptr<mir::graphics::DisplayConfiguration const> const& base_config) override;

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(1, 5, 0)
    void session_configuration_applied(std::shared_ptr<mir::scene::Session> const& session,
        std::shared_ptr<mir::graphics::DisplayConfiguration> const& config) override;

    void session_configuration_removed(std::shared_ptr<mir::scene::Session> const& session);

    void configuration_updated_for_session(
        std::shared_ptr<mir::scene::Session> const&,
        std::shared_ptr<mir::graphics::DisplayConfiguration const> const&) override {};
#else
    void session_configuration_applied(std::shared_ptr<mir::frontend::Session> const& session,
        std::shared_ptr<mir::graphics::DisplayConfiguration> const& config) override;

    void session_configuration_removed(std::shared_ptr<mir::frontend::Session> const& session);
#endif

    void configuration_failed(
            std::shared_ptr<mir::graphics::DisplayConfiguration const> const& attempted,
            std::exception const& error) override;

    void catastrophic_configuration_error(
            std::shared_ptr<mir::graphics::DisplayConfiguration const> const& failed_fallback,
            std::exception const& error);

Q_SIGNALS:
    void initialConfiguration(std::shared_ptr<mir::graphics::DisplayConfiguration const> const& config);
    void configurationApplied(std::shared_ptr<mir::graphics::DisplayConfiguration const> const& config);
    void baseConfigurationUpdated(std::shared_ptr<mir::graphics::DisplayConfiguration const> const& config);
};

#endif // MIRDISPLAYCONFIGURATIONOBSERVER_H
