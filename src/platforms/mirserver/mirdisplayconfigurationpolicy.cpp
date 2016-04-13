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

#include "mirdisplayconfigurationpolicy.h"

#include <mir/graphics/display_configuration.h>
#include <mir/geometry/point.h>

#include <qglobal.h>
#include <QByteArray>

namespace mg = mir::graphics;

#define ENV_GRID_UNIT_PX "GRID_UNIT_PX"
#define DEFAULT_GRID_UNIT_PX 8

namespace {
static float getenvFloat(const char* name, float defaultValue)
{
    QByteArray stringValue = qgetenv(name);
    bool ok;
    float value = stringValue.toFloat(&ok);
    return ok ? value : defaultValue;
}
} //namespace

MirDisplayConfigurationPolicy::MirDisplayConfigurationPolicy(
        const std::shared_ptr<mir::graphics::DisplayConfigurationPolicy> &wrapped)
    : m_wrapped(wrapped)
{
    float gridUnit = DEFAULT_GRID_UNIT_PX;
    if (qEnvironmentVariableIsSet(ENV_GRID_UNIT_PX)) {
        gridUnit = getenvFloat(ENV_GRID_UNIT_PX, DEFAULT_GRID_UNIT_PX);
    }
    m_defaultScale = gridUnit / DEFAULT_GRID_UNIT_PX;
}

void MirDisplayConfigurationPolicy::apply_to(mg::DisplayConfiguration &conf)
{
    int nextTopLeftPosition = 0;

    m_wrapped->apply_to(conf);

    //TODO: scan through saved configurations and select matching one to apply

    // if LVDS panel, guess it is a phone/tablet
    bool phoneDetected = false;
    int screenCount = 0;
    conf.for_each_output(
        [&](const mg::DisplayConfigurationOutput &output )
        {
            if (output.connected && output.used) {
                screenCount++;

                if (output.type == mg::DisplayConfigurationOutputType::lvds) {
                    phoneDetected = true;
                }
            }
        });

    conf.for_each_output(
        [&](mg::UserDisplayConfigurationOutput &output)
        {
            if (!output.connected || !output.used) {
                return;
            }

            output.top_left = mir::geometry::Point{nextTopLeftPosition, 0};
            nextTopLeftPosition += output.modes[output.current_mode_index].size.width.as_int();

            if (phoneDetected) {
                if (screenCount == 1 || output.type == mg::DisplayConfigurationOutputType::lvds) {
                    output.form_factor = mir_form_factor_phone;
                    output.scale = m_defaultScale;
                } else { // screenCount > 1 && output.type != lvds
                    output.form_factor = mir_form_factor_monitor;
                    output.scale = 1;
                }
            } else { // desktop
                output.form_factor = mir_form_factor_monitor;
                output.scale = m_defaultScale; // probably 1 on desktop anyway.
            }
        });
}
