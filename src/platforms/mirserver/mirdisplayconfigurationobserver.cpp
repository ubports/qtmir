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

#include "mirdisplayconfigurationobserver.h"
#include "logging.h"

namespace mg = mir::graphics;
namespace mf = mir::frontend;

MirDisplayConfigurationObserver::MirDisplayConfigurationObserver(QObject *parent)
    : QObject(parent)
{
}

void MirDisplayConfigurationObserver::initial_configuration(const std::shared_ptr<const mg::DisplayConfiguration> &config)
{
    qCDebug(QTMIR_SCREENS) << "MirDisplayConfigurationObserver::initial_configuration";
    Q_EMIT initialConfiguration(config);
}

void MirDisplayConfigurationObserver::configuration_applied(const std::shared_ptr<const mg::DisplayConfiguration> &config)
{
    qCDebug(QTMIR_SCREENS) << "MirDisplayConfigurationObserver::configuration_applied";
    Q_EMIT configurationApplied(config);
}

void MirDisplayConfigurationObserver::base_configuration_updated(const std::shared_ptr<const mg::DisplayConfiguration> &base_config)
{
    qCDebug(QTMIR_SCREENS) << "MirDisplayConfigurationObserver::base_configuration_updated";
    Q_EMIT baseConfigurationUpdated(base_config);
}

void MirDisplayConfigurationObserver::session_configuration_applied(const std::shared_ptr<mf::Session> &,
                                                                    const std::shared_ptr<mg::DisplayConfiguration> &)
{
}

void MirDisplayConfigurationObserver::session_configuration_removed(const std::shared_ptr<mf::Session> &)
{
}

void MirDisplayConfigurationObserver::configuration_failed(const std::shared_ptr<const mg::DisplayConfiguration> &,
                                                           const std::exception &)
{
}

void MirDisplayConfigurationObserver::catastrophic_configuration_error(const std::shared_ptr<const mg::DisplayConfiguration> &,
                                                                       const std::exception &)
{
}
