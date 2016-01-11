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

namespace mg = mir::graphics;

MirDisplayConfigurationPolicy::MirDisplayConfigurationPolicy(
        const std::shared_ptr<mir::graphics::DisplayConfigurationPolicy> &wrapped)
    : m_wrapped(wrapped)
{
}

void MirDisplayConfigurationPolicy::apply_to(mg::DisplayConfiguration& conf)
{
    int nextTopLeftPosition = 0;

    m_wrapped->apply_to(conf);

    //TODO: scan through saved configurations and select matching one to apply

    conf.for_each_output(
        [&](mg::UserDisplayConfigurationOutput& output)
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
