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
#include <QVector>

#include "screentypes.h"
#include <mir_toolkit/common.h>


struct CustomScreenConfiguration
{
    bool valid{false};
    qtmir::OutputId id;

    bool used;
    QPoint topLeft;
    uint32_t currentModeIndex;
    MirPowerMode powerMode;
    MirOrientation orientation;
    float scale;
    MirFormFactor formFactor;

    // To read additional readonly state, consult the Screen
};

typedef QVector<CustomScreenConfiguration> CustomScreenConfigurationList;

#endif // CUSTOMSCREENCONFIGURATION_H
