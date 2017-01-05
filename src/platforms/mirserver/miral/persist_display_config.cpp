/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "persist_display_config.h"

#include <mir/graphics/display_configuration_policy.h>
#include <mir/server.h>

namespace mg = mir::graphics;

namespace
{
struct PersistDisplayConfigPolicy
{
    PersistDisplayConfigPolicy() = default;
    virtual ~PersistDisplayConfigPolicy() = default;
    PersistDisplayConfigPolicy(PersistDisplayConfigPolicy const&) = delete;
    auto operator=(PersistDisplayConfigPolicy const&) -> PersistDisplayConfigPolicy& = delete;

    void apply_to(mg::DisplayConfiguration& conf, mg::DisplayConfigurationPolicy& wrapped);
    void save_config(mg::DisplayConfiguration const& base_conf);
};

struct DisplayConfigurationPolicyAdapter : mg::DisplayConfigurationPolicy
{
    DisplayConfigurationPolicyAdapter(
        std::shared_ptr<PersistDisplayConfigPolicy> const& self,
        std::shared_ptr<mg::DisplayConfigurationPolicy> const& wrapped) :
        self{self}, wrapped{wrapped}
    {}

    void apply_to(mg::DisplayConfiguration& conf) override
    {
        self->apply_to(conf, *wrapped);
    }

    std::shared_ptr<PersistDisplayConfigPolicy> const self;
    std::shared_ptr<mg::DisplayConfigurationPolicy> const wrapped;
};
}

struct qtmir::miral::PersistDisplayConfig::Self : PersistDisplayConfigPolicy {};

qtmir::miral::PersistDisplayConfig::PersistDisplayConfig() :
    self{std::make_shared<Self>()}
{
}

qtmir::miral::PersistDisplayConfig::~PersistDisplayConfig() = default;

qtmir::miral::PersistDisplayConfig::PersistDisplayConfig(PersistDisplayConfig const&) = default;

auto qtmir::miral::PersistDisplayConfig::operator=(PersistDisplayConfig const&) -> PersistDisplayConfig& = default;

void qtmir::miral::PersistDisplayConfig::operator()(mir::Server& server)
{
    server.wrap_display_configuration_policy(
        [this](std::shared_ptr<mg::DisplayConfigurationPolicy> const& wrapped)
        -> std::shared_ptr<mg::DisplayConfigurationPolicy>
        {
            return std::make_shared<DisplayConfigurationPolicyAdapter>(self, wrapped);
        });

    // TODO create an adapter to detect changes to base config
}

void PersistDisplayConfigPolicy::apply_to(
    mg::DisplayConfiguration& conf,
    mg::DisplayConfigurationPolicy& wrapped)
{
    // TODO if the h/w profile (by some definition) has changed, then apply saved config (if any).
    // TODO Otherwise...
    wrapped.apply_to(conf);
}

void PersistDisplayConfigPolicy::save_config(mg::DisplayConfiguration const& /*base_conf*/)
{
    // TODO save display config options against the h/w profile
}
