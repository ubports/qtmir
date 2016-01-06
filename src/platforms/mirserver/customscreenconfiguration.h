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

#ifndef CUSTOMSCREENCONFIGURATION_H
#define CUSTOMSCREENCONFIGURATION_H

#include <QPoint>

#include <mir/graphics/display_configuration.h>

struct CustomScreenConfiguration
{
    mir::graphics::DisplayConfigurationOutputId id;
    QPoint topLeft;
    uint32_t modeIndex;
    MirPowerMode powerMode;
    MirOrientation orientation;
    float scale;
    MirFormFactor formFactor;
};

#endif // CUSTOMSCREENCONFIGURATION_H
