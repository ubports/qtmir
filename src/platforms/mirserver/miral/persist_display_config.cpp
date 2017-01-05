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

    void apply_to(mg::DisplayConfiguration& conf, mg::DisplayConfigurationPolicy& default_policy);
    void save_config(mg::DisplayConfiguration const& base_conf);
};

struct DisplayConfigurationPolicyAdapter : mg::DisplayConfigurationPolicy
{
    DisplayConfigurationPolicyAdapter(
        std::shared_ptr<PersistDisplayConfigPolicy> const& self,
        std::shared_ptr<mg::DisplayConfigurationPolicy> const& wrapped) :
        self{self}, default_policy{wrapped}
    {}

    void apply_to(mg::DisplayConfiguration& conf) override
    {
        self->apply_to(conf, *default_policy);
    }

    std::shared_ptr<PersistDisplayConfigPolicy> const self;
    std::shared_ptr<mg::DisplayConfigurationPolicy> const default_policy;
};
}

struct qtmir::miral::PersistDisplayConfig::Self : PersistDisplayConfigPolicy
{
    Self() = default;
    Self(DisplayConfigurationPolicyWrapper const& custom_wrapper) :
        custom_wrapper{custom_wrapper} {}

    DisplayConfigurationPolicyWrapper const custom_wrapper =
        [](const std::shared_ptr<mg::DisplayConfigurationPolicy> &wrapped) { return wrapped; };
};

qtmir::miral::PersistDisplayConfig::PersistDisplayConfig() :
    self{std::make_shared<Self>()}
{
}

qtmir::miral::PersistDisplayConfig::PersistDisplayConfig(DisplayConfigurationPolicyWrapper const& custom_wrapper) :
    self{std::make_shared<Self>(custom_wrapper)}
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
            return std::make_shared<DisplayConfigurationPolicyAdapter>(self, self->custom_wrapper(wrapped));
        });

    // TODO create an adapter to detect changes to base config
    // Up to Mir-0.25 this is only possible client-side and gives a different configuration API
    // This is such a PITA that I'll first make changes to lp:mir and hope to land them in 0.26 - alan_g
    (void)&PersistDisplayConfigPolicy::save_config; // Fake "using" the function for now
}

void PersistDisplayConfigPolicy::apply_to(
    mg::DisplayConfiguration& conf,
    mg::DisplayConfigurationPolicy& default_policy)
{
    // TODO if the h/w profile (by some definition) has changed, then apply corresponding saved config (if any).
    // TODO Otherwise...
    default_policy.apply_to(conf);
}

void PersistDisplayConfigPolicy::save_config(mg::DisplayConfiguration const& /*base_conf*/)
{
    // TODO save display config options against the h/w profile
}
