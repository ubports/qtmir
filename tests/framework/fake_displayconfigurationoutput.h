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

#ifndef FAKE_DISPLAYCONFIGURATIONOUTPUT_H
#define FAKE_DISPLAYCONFIGURATIONOUTPUT_H

#include <mir/graphics/display_configuration.h>

#include <vector>

namespace mg = mir::graphics;
namespace geom = mir::geometry;

const mg::DisplayConfigurationOutput fakeOutput1
{
    mg::DisplayConfigurationOutputId{3},
    mg::DisplayConfigurationCardId{2},
    mg::DisplayConfigurationOutputType::dvid,
    {
        mir_pixel_format_abgr_8888
    },
    {
        {geom::Size{100, 200}, 60.0},
        {geom::Size{100, 200}, 59.0},
        {geom::Size{150, 200}, 59.0}
    },
    0,
    geom::Size{1111, 2222},
    true,
    true,
    geom::Point(),
    2,
    mir_pixel_format_abgr_8888,
    mir_power_mode_on,
    mir_orientation_normal,
    1.0f,
    mir_form_factor_unknown,
    mir_subpixel_arrangement_unknown,
    {},
    mir_output_gamma_unsupported,
    {}
#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 27, 0)
    ,{}
#endif
};

const mg::DisplayConfigurationOutput fakeOutput2
{
    mg::DisplayConfigurationOutputId{2},
    mg::DisplayConfigurationCardId{4},
    mg::DisplayConfigurationOutputType::lvds,
    {
        mir_pixel_format_xbgr_8888
    },
    {
        {geom::Size{800, 1200}, 90.0},
        {geom::Size{1600, 2400}, 60.0},
        {geom::Size{1500, 2000}, 75.0}
    },
    0,
    geom::Size{1000, 2000},
    true,
    true,
    geom::Point(500, 600),
    2,
    mir_pixel_format_xbgr_8888,
    mir_power_mode_on,
    mir_orientation_left,
    1.0f,
    mir_form_factor_unknown,
    mir_subpixel_arrangement_unknown,
    {},
    mir_output_gamma_unsupported,
    {}
#if MIR_SERVER_VERSION >= MIR_VERSION_NUMBER(0, 27, 0)
    ,{}
#endif
};

#endif // FAKE_DISPLAYCONFIGURATIONOUTPUT_H
