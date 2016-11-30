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

// qtmir
#include "qtmir/displayconfigurationpolicy.h"
#include "qmirserver.h"

// mir
#include <mir/geometry/point.h>
#include <mir/graphics/display_configuration.h>

#include <qglobal.h>
#include <QByteArray>

#define ENV_GRID_UNIT_PX "GRID_UNIT_PX"
#define DEFAULT_GRID_UNIT_PX 8

namespace mg = mir::graphics;

namespace qtmir
{
namespace
{

float getenvFloat(const char* name, float defaultValue)
{
    QByteArray stringValue = qgetenv(name);
    bool ok;
    float value = stringValue.toFloat(&ok);
    return ok ? value : defaultValue;
}

} // namespace

struct DisplayConfigurationPolicy::Private
{
    Private()
    {
        float gridUnit = DEFAULT_GRID_UNIT_PX;
        if (qEnvironmentVariableIsSet(ENV_GRID_UNIT_PX)) {
            gridUnit = getenvFloat(ENV_GRID_UNIT_PX, DEFAULT_GRID_UNIT_PX);
        }
        m_defaultScale = gridUnit / DEFAULT_GRID_UNIT_PX;
    }

    float m_defaultScale;
};

DisplayConfigurationPolicy::DisplayConfigurationPolicy() :
    d(new DisplayConfigurationPolicy::Private)
{
}

void DisplayConfigurationPolicy::apply_to(mg::DisplayConfiguration& conf)
{
    int nextTopLeftPosition = 0;

    //TODO: scan through saved configurations and select matching one to apply

    // We want to apply a particular display config policy when connecting an external display
    // to a phone/tablet. We don't have a reliable way to distinguish a phone/tablet display
    // from a laptop display as yet.
    //
    // Best we can do currently is guess that LVDS panel implies a phone/tablet
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
                    output.scale = d->m_defaultScale;
                } else { // screenCount > 1 && output.type != lvds
                    output.form_factor = mir_form_factor_monitor;
                    output.scale = 1;
                }
            } else { // desktop
                output.form_factor = mir_form_factor_monitor;
                output.scale = d->m_defaultScale; // probably 1 on desktop anyway.
            }
        });
}


struct BasicSetDisplayConfigurationPolicy::Self
{
    Self(DisplayConfigurationPolicyWrapper const& builder) :
        builder{builder} {}

    DisplayConfigurationPolicyWrapper builder;
};

BasicSetDisplayConfigurationPolicy::BasicSetDisplayConfigurationPolicy(DisplayConfigurationPolicyWrapper const& builder)
    : self(std::make_shared<Self>(builder))
{
}

void BasicSetDisplayConfigurationPolicy::operator()(QMirServer& server)
{
    server.wrapDisplayConfigurationPolicy(self->builder);
}

} // namespace qtmir
