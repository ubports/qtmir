/*
 * Copyright (C) 2015-2016 Canonical, Ltd.
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

#include "screensmodel.h"
#include "stub_screen.h"

struct TestableScreensModel : public ScreensModel
{
    Q_OBJECT

public:
    Screen *createScreen(const mir::graphics::DisplayConfigurationOutput &output) const override
    {
        return new StubScreen(output);
    }

    void do_init(const std::shared_ptr<mir::graphics::Display> &display,
                 const std::shared_ptr<mir::compositor::Compositor> &compositor)
    {
        init(display, compositor);
    }

    void do_terminate() { terminate(); }
};
