/*
 * Copyright (C) 2015 Canonical, Ltd.
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

using namespace qtmir;

ScreensController::ScreensController(
        const std::shared_ptr<mir::graphics::Display> &display,
        const std::shared_ptr<mir::shell::DisplayConfigurationController> &controller,
        QObject *parent)
    : m_display(display)
    , m_displayConfigurationController(controller)
    , QObject(parent)
{
}

CustomScreenConfiguration ScreensController::getConfigurationFor(Screen *screen)
{
    CustomScreenConfiguration config {
        screen->outputId(),
        screen->geometry().topLeft(),
        screen->currentModeIndex(),
        screen->powerMode(),
        mir_orientation_normal, //screen->orientation(),
        screen->scale(),
        screen->formFactor()
    };
    return config;
}

void ScreensController::queueConfigurationChange(CustomScreenConfiguration config)
{
    m_configurationQueue.append(config);
}

void ScreensController::applyConfigurationChanges()
{
    auto controller = m_displayConfigurationController.lock();
    auto display = m_display.lock();

    if (m_configurationQueue.isEmpty() || !controller || !display) {
        return;
    }

    auto displayConfiguration = display->configuration();

    Q_FOREACH (auto config, m_configurationQueue) {
        displayConfiguration->for_each_output(
            [&config](mg::UserDisplayConfigurationOutput &outputConfig)
            {
                if (config.id == outputConfig.id) {
                    outputConfig.current_mode_index = config.modeIndex;
                    outputConfig.power_mode = config.powerMode;
                    outputConfig.scale = config.scale;
                    outputConfig.form_factor = config.formFactor;
                }
            });
    }

    controller->set_base_configuration(std::move(displayConfiguration));
    m_configurationQueue.clear();
}
