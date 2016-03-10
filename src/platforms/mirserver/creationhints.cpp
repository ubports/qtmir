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

#include "creationhints.h"

using namespace qtmir;

inline const char* shellChromeToString(Mir::ShellChrome chrome) {
    switch (chrome) {
        case Mir::ShellChrome::NormalChrome:
            return "normal";
        case Mir::ShellChrome::LowChrome:
            return "low";
    }
    return "unknown";
}

CreationHints::CreationHints(const mir::scene::SurfaceCreationParameters &params)
{
    minWidth = params.min_width.is_set() ? params.min_width.value().as_int() : 0;
    maxWidth = params.max_width.is_set() ? params.max_width.value().as_int() : 0;

    minHeight = params.min_height.is_set() ? params.min_height.value().as_int() : 0;
    maxHeight = params.max_height.is_set() ? params.max_height.value().as_int() : 0;

    widthIncrement = params.width_inc.is_set() ? params.width_inc.value().as_int() : 0;
    heightIncrement = params.height_inc.is_set() ? params.height_inc.value().as_int() : 0;

    if (params.shell_chrome.is_set()) {
        switch (params.shell_chrome.value()) {
            case mir_shell_chrome_normal:
            default:
                shellChrome = Mir::ShellChrome::NormalChrome;
                break;
            case mir_shell_chrome_low:
                shellChrome = Mir::ShellChrome::NormalChrome;
                break;
        }
    }
}

QString CreationHints::toString() const
{
    return QString("CreationHints(minW=%1,minH=%2,maxW=%3,maxH=%4,wIncr=%5,hInc=%6,shellChrome=%7)")
        .arg(minWidth)
        .arg(minHeight)
        .arg(maxWidth)
        .arg(maxHeight)
        .arg(widthIncrement)
        .arg(heightIncrement)
        .arg(shellChromeToString(shellChrome));
}
