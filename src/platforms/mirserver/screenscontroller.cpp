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

#include "screenscontroller.h"
#include "screen.h"

// Mir
#include <mir/graphics/display.h>
#include <mir/shell/display_configuration_controller.h>
#include <mir/geometry/point.h>

namespace mg = mir::graphics;

ScreensController::ScreensController(const QSharedPointer<ScreensModel> &model,
        const std::shared_ptr<mir::graphics::Display> &display,
        const std::shared_ptr<mir::graphics::DisplayConfigurationPolicy> &policy,
        const std::weak_ptr<mir::shell::DisplayConfigurationController> &controller,
        QObject *parent)
    : QObject(parent)
    , m_screensModel(model)
    , m_display(display)
    , m_wrappedDisplayConfigurationPolicy(policy)
    , m_displayConfigurationController(controller)
{
}

CustomScreenConfigurationList ScreensController::configuration() const
{
    CustomScreenConfigurationList list;

    Q_FOREACH(auto screen, m_screensModel->screens()) {
        list.append(
            CustomScreenConfiguration {
                        screen->outputId(),
                        screen->geometry().topLeft(),
                        screen->currentModeIndex(),
                        screen->powerMode(),
                        mir_orientation_normal, //screen->orientation(), disable for now
                        screen->scale(),
                        screen->formFactor()
            });
    }
    return list;
}

bool ScreensController::setConfiguration(CustomScreenConfigurationList newConfig)
{
    using namespace mir::geometry;
    auto controller = m_displayConfigurationController.lock();

    if (!controller) {
        return false;
    }

    auto displayConfiguration = m_display->configuration();

    Q_FOREACH (auto config, newConfig) {
        displayConfiguration->for_each_output(
            [&config](mg::UserDisplayConfigurationOutput &outputConfig)
            {
                if (config.id == outputConfig.id) {
                    outputConfig.current_mode_index = config.currentModeIndex;
                    outputConfig.top_left = Point{ X{config.topLeft.x()}, Y{config.topLeft.y()}};
                    outputConfig.power_mode = config.powerMode;
//                    outputConfig.orientation = config.orientation; // disabling for now
                    outputConfig.scale = config.scale;
                    outputConfig.form_factor = config.formFactor;
                }
            });
    }

    if (!displayConfiguration->valid()) {
        return false;
    }

    controller->set_base_configuration(std::move(displayConfiguration));

    //TODO: Save this configuration for future use
    return true;
}

void ScreensController::apply_to(mg::DisplayConfiguration& conf)
{
    int nextTopLeftPosition = 0;

    m_wrappedDisplayConfigurationPolicy->apply_to(conf);

    //TODO: scan through saved configurations and select matching one to apply

    conf.for_each_output(
        [&](mg::UserDisplayConfigurationOutput &output)
        {
            if (output.connected && output.used) {
                output.top_left = mir::geometry::Point{nextTopLeftPosition, 0};
                nextTopLeftPosition += output.modes[output.preferred_mode_index].size.width.as_int();
            }

            if (output.type == mg::DisplayConfigurationOutputType::hdmia
                    || output.type == mg::DisplayConfigurationOutputType::hdmib) {
                output.form_factor = mir_form_factor_monitor;
                output.scale = 2;
            } else {
                output.form_factor = mir_form_factor_phone;
                output.scale = 1;
            }
        });
}
