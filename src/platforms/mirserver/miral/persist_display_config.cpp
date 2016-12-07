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
#include "qtmir/miral/display_configuration_policy.h"
#include "qtmir/miral/display_configuration_storage.h"
#include "qtmir/miral/edid.h"

#include <mir/graphics/display_configuration_policy.h>
#include <mir/graphics/display_configuration.h>
#include <mir/server.h>
#include <mir/version.h>

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 26, 0)
#include <mir/graphics/display_configuration_observer.h>
#include <mir/observer_registrar.h>
#endif

namespace mg = mir::graphics;

namespace
{
struct PersistDisplayConfigPolicy
{
    PersistDisplayConfigPolicy(std::shared_ptr<miral::DisplayConfigurationStorage> const& storage) :
        storage(storage) {}
    virtual ~PersistDisplayConfigPolicy() = default;
    PersistDisplayConfigPolicy(PersistDisplayConfigPolicy const&) = delete;
    auto operator=(PersistDisplayConfigPolicy const&) -> PersistDisplayConfigPolicy& = delete;

    void apply_to(mg::DisplayConfiguration& conf,
                  miral::DisplayConfigurationPolicy& default_policy,
                  miral::DisplayConfigurationPolicy& wrapped_policy);
    void save_config(mg::DisplayConfiguration const& base_conf);

    std::shared_ptr<miral::DisplayConfigurationStorage> storage;
};

struct MiralWrappedMirDisplayConfigurationPolicy : miral::DisplayConfigurationPolicy
{
    MiralWrappedMirDisplayConfigurationPolicy(std::shared_ptr<mg::DisplayConfigurationPolicy> const& self) :
        self{self}
    {}

    void apply_to(mg::DisplayConfiguration& conf) override
    {
        self->apply_to(conf);
    }

    std::shared_ptr<mg::DisplayConfigurationPolicy> self;
};

struct DisplayConfigurationPolicyAdapter : mg::DisplayConfigurationPolicy
{
    DisplayConfigurationPolicyAdapter(
        std::shared_ptr<PersistDisplayConfigPolicy> const& self,
        std::shared_ptr<miral::DisplayConfigurationPolicy> const& wrapper,
            std::shared_ptr<miral::DisplayConfigurationPolicy> const& wrapped) :
        self{self}, custom_policy{wrapper}, wrapped_policy{wrapped}
    {}

    void apply_to(mg::DisplayConfiguration& conf) override
    {
        self->apply_to(conf, *custom_policy, *wrapped_policy);
    }

    std::shared_ptr<PersistDisplayConfigPolicy> const self;
    std::shared_ptr<miral::DisplayConfigurationPolicy> const custom_policy;
    std::shared_ptr<miral::DisplayConfigurationPolicy> const wrapped_policy;
};

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 26, 0)
struct DisplayConfigurationObserver : mg::DisplayConfigurationObserver
{
    void initial_configuration(std::shared_ptr<mg::DisplayConfiguration const> const& /*config*/) override {}

    void configuration_applied(std::shared_ptr<mg::DisplayConfiguration const> const& /*config*/) override {}

    void configuration_failed(
        std::shared_ptr<mg::DisplayConfiguration const> const& /*attempted*/,
        std::exception const& /*error*/) override {}

    void catastrophic_configuration_error(
        std::shared_ptr<mg::DisplayConfiguration const> const& /*failed_fallback*/,
        std::exception const& /*error*/) override {}
};
#else
struct DisplayConfigurationObserver { };
#endif
}

struct miral::PersistDisplayConfig::Self : PersistDisplayConfigPolicy, DisplayConfigurationObserver
{
    Self(std::shared_ptr<DisplayConfigurationStorage> const& storage,
         DisplayConfigurationPolicyWrapper const& custom_wrapper) :
        PersistDisplayConfigPolicy(storage),
        custom_wrapper{custom_wrapper} {}

    DisplayConfigurationPolicyWrapper const custom_wrapper;

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 26, 0)
    void base_configuration_updated(std::shared_ptr<mg::DisplayConfiguration const> const& base_config) override
    {
        save_config(*base_config);
    }
#endif
};

miral::PersistDisplayConfig::PersistDisplayConfig(std::shared_ptr<DisplayConfigurationStorage> const& storage,
                                                  DisplayConfigurationPolicyWrapper const& custom_wrapper) :
    self{std::make_shared<Self>(storage, custom_wrapper)}
{
}

miral::PersistDisplayConfig::~PersistDisplayConfig() = default;

miral::PersistDisplayConfig::PersistDisplayConfig(PersistDisplayConfig const&) = default;

auto miral::PersistDisplayConfig::operator=(PersistDisplayConfig const&) -> PersistDisplayConfig& = default;

void miral::PersistDisplayConfig::operator()(mir::Server& server)
{
    server.wrap_display_configuration_policy(
        [this](std::shared_ptr<mg::DisplayConfigurationPolicy> const& wrapped)
        -> std::shared_ptr<mg::DisplayConfigurationPolicy>
        {
            auto custom_wrapper = self->custom_wrapper();
            return std::make_shared<DisplayConfigurationPolicyAdapter>(self,
                                                                       custom_wrapper,
                                                                       std::make_shared<MiralWrappedMirDisplayConfigurationPolicy>(wrapped));
        });

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 26, 0)
    server.add_init_callback([this, &server]
        { server.the_display_configuration_observer_registrar()->register_interest(self); });
#else
    // Up to Mir-0.25 detecting changes to the base display config is only possible client-side
    // (and gives a different configuration API)
    // If we decide implementing this is necessary for earlier Mir versions then this is where to plumb it in.
    (void)&PersistDisplayConfigPolicy::save_config; // Fake "using" the function for now
#endif
}

void PersistDisplayConfigPolicy::apply_to(
    mg::DisplayConfiguration& conf,
    miral::DisplayConfigurationPolicy& default_policy,
    miral::DisplayConfigurationPolicy& wrapped_policy)
{
    if (storage) {
        conf.for_each_output([this](mg::UserDisplayConfigurationOutput& output) {

            miral::Edid edid;
            edid.parse_data(reinterpret_cast<std::vector<uint8_t> const&>(output.edid));
            if (edid.has_error) {
                return;
            }

            // TODO if the h/w profile (by some definition) has changed, then apply corresponding saved config (if any).
            // TODO Otherwise...

            miral::DisplayOutputConfiguration config;
            if (storage->load(edid, config)) {

                int mode_index = output.current_mode_index;
                int i = 0;
                // Find the mode index which supports the saved size.
                for(auto iter = output.modes.cbegin(); iter != output.modes.cend(); ++iter, i++) {
                    if ((*iter).size == config.extents.size) {
                        mode_index = i;
                    }
                }

                output.current_mode_index = mode_index;
                output.orientation = config.orientation;
                output.used = config.used;
                output.form_factor = config.form_factor;
                output.scale = config.scale;
            }
        });
    }

    wrapped_policy.apply_to(conf);
    default_policy.apply_to(conf);
}

void PersistDisplayConfigPolicy::save_config(mg::DisplayConfiguration const& conf)
{
    // TODO save display config options against the h/w profile
    if (storage) {
        conf.for_each_output([this](mg::DisplayConfigurationOutput const& output) {

            miral::Edid edid;
            edid.parse_data(reinterpret_cast<std::vector<uint8_t> const&>(output.edid));
            if (edid.has_error) {
                return;
            }

            miral::DisplayOutputConfiguration config;
            config.extents = output.extents();
            config.form_factor = output.form_factor;
            config.orientation = output.orientation;
            config.scale = output.scale;
            config.used = output.used;

            storage->save(edid, config);
        });
    }
}
