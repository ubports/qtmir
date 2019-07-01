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

#ifndef STUBSCREEN_H
#define STUBSCREEN_H

#include "platformscreen.h"
#include "orientationsensor.h"

class StubScreen : public PlatformScreen
{
    Q_OBJECT
public:
    StubScreen(const mir::graphics::DisplayConfigurationOutput &output) : PlatformScreen(output, std::make_shared<OrientationSensor>()) {}

    void makeCurrent() { PlatformScreen::makeCurrent(); }
};

#endif // STUBSCREEN_H
