/*
 * Copyright (C) 2020 UBports Foundation
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

#include <memory>

#include "qtwindowmanager.h"
#include "urlbuilderdispatcher.h"
#include "services.h"

#include <wayland-generated/qt_windowmanager_wrapper.h>

#include <QDebug>

using namespace mir::wayland;
using namespace miral;

namespace
{
class QtWindowManager : public Windowmanager
{
public:
    using Windowmanager::Windowmanager;

    QtWindowManager(struct wl_resource*);

    void open_url(uint32_t remaining, std::string const& url) override {
        mBuilder->urlInput(remaining, url);
    }
private:
    qtmir::URLBuilderDispatcher* mBuilder;
};

QtWindowManager::QtWindowManager(
    struct wl_resource* resource) :
    Windowmanager{resource, Version<1>{}},
    mBuilder{new qtmir::URLBuilderDispatcher{std::make_shared<Services>()}}
{
}

class MyGlobal : public QtWindowManager::Global
{
public:
    explicit MyGlobal(wl_display* display);
    
    void bind(wl_resource* new_windowmanager) override;
};

}

MyGlobal::MyGlobal(wl_display* display) :
    Global(display, Version<1>{})
{
}

void MyGlobal::bind(wl_resource* new_windowmanager)
{
    new QtWindowManager{new_windowmanager};
}

auto qtmir::qtWindowmanagerExtension() -> WaylandExtensions::Builder {
    return {
        Windowmanager::interface_name,
        [](WaylandExtensions::Context const* context) {
            return std::make_shared<MyGlobal>(context->display());
        }
    };
}
