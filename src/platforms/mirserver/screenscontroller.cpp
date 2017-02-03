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
#include "platformscreen.h"
#include "screensmodel.h"

// Mir
#include <mir/graphics/display.h>
#include <mir/shell/display_configuration_controller.h>
#include <mir/geometry/point.h>

namespace mg = mir::graphics;

ScreensController::ScreensController(const QSharedPointer<ScreensModel> &model,
        const std::shared_ptr<mir::graphics::Display> &display,
        const std::shared_ptr<mir::shell::DisplayConfigurationController> &controller,
        QObject *parent)
    : QObject(parent)
    , m_screensModel(model)
    , m_display(display)
    , m_displayConfigurationController(controller)
{
}

CustomScreenConfigurationList ScreensController::configuration()
{
    CustomScreenConfigurationList list;

    Q_FOREACH(auto screen, m_screensModel->screens()) {
        list.append(
            CustomScreenConfiguration {
                        true,
                        screen->outputId(),
                        screen->used(),
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

bool ScreensController::setConfiguration(const CustomScreenConfigurationList &newConfig)
{
    using namespace mir::geometry;

    auto displayConfiguration = m_display->configuration();

    Q_FOREACH (const auto &config, newConfig) {
        displayConfiguration->for_each_output(
            [&config](mg::UserDisplayConfigurationOutput &outputConfig)
            {
                if (config.id == outputConfig.id) {
                    outputConfig.used = config.used;
                    outputConfig.top_left = Point{ X{config.topLeft.x()}, Y{config.topLeft.y()}};
                    outputConfig.current_mode_index = config.currentModeIndex;
                    outputConfig.power_mode = config.powerMode;
//                    outputConfig.orientation = config.orientation; // disabling for now
                    outputConfig.scale = config.scale;
                    outputConfig.form_factor = static_cast<MirFormFactor>(config.formFactor);
                }
            });
    }

    if (!displayConfiguration->valid()) {
        return false;
    }

    m_displayConfigurationController->set_base_configuration(std::move(displayConfiguration));
    return true;
}

CustomScreenConfiguration ScreensController::outputConfiguration(qtmir::OutputId outputId)
{
    auto displayConfiguration = m_display->configuration();
    CustomScreenConfiguration config;

    displayConfiguration->for_each_output(
        [&config, outputId](mg::UserDisplayConfigurationOutput &outputConfig)
        {
            if (outputConfig.id == outputId) {
                config.valid = true;
                config.id = outputConfig.id;
                config.used = outputConfig.used;
                config.topLeft = QPoint{outputConfig.top_left.x.as_int(), outputConfig.top_left.y.as_int()};
                config.currentModeIndex = outputConfig.current_mode_index;
                config.powerMode = outputConfig.power_mode;
                config.orientation = outputConfig.orientation;
                config.scale = outputConfig.scale;
                config.formFactor = static_cast<qtmir::FormFactor>(outputConfig.form_factor);
            }
    });
    return config;
}

bool ScreensController::setOutputConfiguration(const CustomScreenConfiguration &newConfig)
{
    using namespace mir::geometry;
    if (!newConfig.valid)
        return false;

    auto displayConfiguration = m_display->configuration();

    displayConfiguration->for_each_output(
        [newConfig](mg::UserDisplayConfigurationOutput &outputConfig)
        {
            if (outputConfig.id == newConfig.id) {
                outputConfig.used = newConfig.used;
                outputConfig.top_left = Point{ X{newConfig.topLeft.x()}, Y{newConfig.topLeft.y()}};
                outputConfig.current_mode_index = newConfig.currentModeIndex;
                outputConfig.power_mode = newConfig.powerMode;
//              outputConfig.orientation = newConfig.orientation; // disabling for now
                outputConfig.scale = newConfig.scale;
                outputConfig.form_factor = static_cast<MirFormFactor>(newConfig.formFactor);
            }
        });

    if (!displayConfiguration->valid()) {
        return false;
    }

    m_displayConfigurationController->set_base_configuration(std::move(displayConfiguration));
    return true;
}
