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
#include "display_configuration_storage.h"
#include "edid.h"

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

    void apply_to(mg::DisplayConfiguration& conf, mg::DisplayConfigurationPolicy& default_policy);
    void save_config(mg::DisplayConfiguration const& base_conf);

    std::shared_ptr<miral::DisplayConfigurationStorage> storage;
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
    Self(std::shared_ptr<DisplayConfigurationStorage> const& storage) :
        PersistDisplayConfigPolicy(storage) {}
    Self(std::shared_ptr<DisplayConfigurationStorage> const& storage,
         DisplayConfigurationPolicyWrapper const& custom_wrapper) :
        PersistDisplayConfigPolicy(storage),
        custom_wrapper{custom_wrapper} {}

    DisplayConfigurationPolicyWrapper const custom_wrapper =
        [](const std::shared_ptr<mg::DisplayConfigurationPolicy> &wrapped) { return wrapped; };

#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 26, 0)
    void base_configuration_updated(std::shared_ptr<mg::DisplayConfiguration const> const& base_config) override
    {
        save_config(*base_config);
    }

    void session_configuration_applied(std::shared_ptr<mir::frontend::Session> const&,
                                       std::shared_ptr<mg::DisplayConfiguration> const&){}
    void session_configuration_removed(std::shared_ptr<mir::frontend::Session> const&)  {}
#endif
};

miral::PersistDisplayConfig::PersistDisplayConfig(std::shared_ptr<DisplayConfigurationStorage> const& storage) :
    self{std::make_shared<Self>(storage)}
{
}

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
            return std::make_shared<DisplayConfigurationPolicyAdapter>(self, self->custom_wrapper(wrapped));
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
    mg::DisplayConfigurationPolicy& default_policy)
{
    default_policy.apply_to(conf);

    if (!storage) {
        throw std::runtime_error("No display configuration storage supplied.");
    }

    conf.for_each_output([this, &conf](mg::UserDisplayConfigurationOutput& output) {

        try {
            miral::Edid edid;
            // FIXME - output.edid should be std::vector<uint8_t>, not std::vector<uint8_t const>
            edid.parse_data(reinterpret_cast<std::vector<uint8_t> const&>(output.edid));

            // TODO if the h/w profile (by some definition) has changed, then apply corresponding saved config (if any).
            // TODO Otherwise...

            miral::DisplayOutputOptions config;
            if (storage->load(edid, config)) {

                if (config.size.is_set()) {
                    int mode_index = output.current_mode_index;
                    int i = 0;
                    // Find the mode index which supports the saved size.
                    for(auto iter = output.modes.cbegin(); iter != output.modes.cend(); ++iter, i++) {
                        if ((*iter).size == config.size.value()) {
                            mode_index = i;
                            break;
                        }
                    }
                    output.current_mode_index = mode_index;
                }

                uint output_index = 0;
                conf.for_each_output([this, &output, config, &output_index](mg::DisplayConfigurationOutput const& find_output) {
                    if (output_index == config.clone_output_index.value()) {
                        output.top_left = find_output.top_left;
                    }
                    output_index++;
                });

                if (config.orientation.is_set()) {output.orientation = config.orientation.value(); }
                if (config.used.is_set()) {output.used = config.used.value(); }
                if (config.form_factor.is_set()) {output.form_factor = config.form_factor.value(); }
                if (config.scale.is_set()) {output.scale = config.scale.value(); }
            }
        } catch (std::runtime_error const&) {
        }
    });
}

void PersistDisplayConfigPolicy::save_config(mg::DisplayConfiguration const& conf)
{
    if (!storage) return;

    conf.for_each_output([this, &conf](mg::DisplayConfigurationOutput const& output) {

        try {
            miral::Edid edid;
            // FIXME - output.edid should be std::vector<uint8_t>, not std::vector<uint8_t const>
            edid.parse_data(reinterpret_cast<std::vector<uint8_t> const&>(output.edid));

            miral::DisplayOutputOptions config;

            uint output_index = 0;
            conf.for_each_output([this, output, &config, &output_index](mg::DisplayConfigurationOutput const& find_output) {
                if (!config.clone_output_index.is_set() && output.top_left == find_output.top_left) {
                    config.clone_output_index = output_index;
                }
                output_index++;
            });

            config.size = output.extents().size;
            config.form_factor = output.form_factor;
            config.orientation = output.orientation;
            config.scale = output.scale;
            config.used = output.used;

            storage->save(edid, config);
        } catch (std::runtime_error const&) {
        }
    });
}
