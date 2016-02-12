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

#include <mir/scene/surface_creation_parameters.h>

#include "sizehints.h"

using namespace qtmir;

SizeHints::SizeHints(const mir::scene::SurfaceCreationParameters &params)
{
    minWidth = params.min_width.is_set() ? params.min_width.value().as_int() : 0;
    maxWidth = params.max_width.is_set() ? params.max_width.value().as_int() : 0;

    minHeight = params.min_height.is_set() ? params.min_height.value().as_int() : 0;
    maxHeight = params.max_height.is_set() ? params.max_height.value().as_int() : 0;

    widthIncrement = params.width_inc.is_set() ? params.width_inc.value().as_int() : 0;
    heightIncrement = params.height_inc.is_set() ? params.height_inc.value().as_int() : 0;
}

QString SizeHints::toString() const
{
    return QString("SizeHints(minW=%1,minH=%2,maxW=%3,maxH=%4,wIncr=%5,hInc=%6)")
        .arg(minWidth)
        .arg(minHeight)
        .arg(maxWidth)
        .arg(maxHeight)
        .arg(widthIncrement)
        .arg(heightIncrement);
}
