/*
 * Copyright © 2015 Canonical Ltd.
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

#include "mirshell.h"
#include "logging.h"
#include "tracepoints.h" // generated from tracepoints.tp

#include <mir/geometry/rectangle.h>
#include <mir/scene/session.h>
#include <mir/scene/surface_creation_parameters.h>
#include <mir/scene/surface.h>
#include <mir/shell/display_layout.h>
#include <mir/shell/window_manager.h>

namespace ms = mir::scene;

QtMirWindowManager::QtMirWindowManager(const std::shared_ptr<mir::shell::DisplayLayout> &displayLayout) :
    m_displayLayout{displayLayout}
{
    qCDebug(QTMIR_MIR_MESSAGES) << "QtMirWindowManager::QtMirWindowManager";
}

void QtMirWindowManager::add_session(std::shared_ptr<ms::Session> const& /*session*/)
{
}

void QtMirWindowManager::remove_session(std::shared_ptr<ms::Session> const& /*session*/)
{
}

auto QtMirWindowManager::add_surface(
    std::shared_ptr<ms::Session> const& session,
    ms::SurfaceCreationParameters const& requestParameters,
    std::function<mir::frontend::SurfaceId(std::shared_ptr<ms::Session> const& session, ms::SurfaceCreationParameters const& params)> const& build)
-> mir::frontend::SurfaceId
{
    tracepoint(qtmirserver, surfacePlacementStart);

    // TODO: Callback unity8 so that it can make a decision on that.
    //       unity8 must bear in mind that the called function will be on a Mir thread though.
    //       The QPA shouldn't be deciding for itself on such things.

    ms::SurfaceCreationParameters placedParameters = requestParameters;

    // Just make it fullscreen for now
    mir::geometry::Rectangle rect{requestParameters.top_left, requestParameters.size};
    m_displayLayout->size_to_output(rect);
    placedParameters.size = rect.size;

    qCDebug(QTMIR_MIR_MESSAGES) << "QtMirWindowManager::add_surface(): size requested ("
                                << requestParameters.size.width.as_int() << "," << requestParameters.size.height.as_int() << ") and placed ("
                                << placedParameters.size.width.as_int() << "," << placedParameters.size.height.as_int() << ")";

    tracepoint(qtmirserver, surfacePlacementEnd);

    return build(session, placedParameters);
}

void QtMirWindowManager::remove_surface(
    std::shared_ptr<ms::Session> const& /*session*/,
    std::weak_ptr<ms::Surface> const& /*surface*/)
{
}

void QtMirWindowManager::add_display(mir::geometry::Rectangle const& /*area*/)
{
}

void QtMirWindowManager::remove_display(mir::geometry::Rectangle const& /*area*/)
{
}

bool QtMirWindowManager::handle_keyboard_event(MirKeyboardEvent const* /*event*/)
{
    return false;
}

bool QtMirWindowManager::handle_touch_event(MirTouchEvent const* /*event*/)
{
    return false;
}

bool QtMirWindowManager::handle_pointer_event(MirPointerEvent const* /*event*/)
{
    return false;
}

int QtMirWindowManager::set_surface_attribute(
    std::shared_ptr<ms::Session> const& /*session*/,
    std::shared_ptr<ms::Surface> const& surface,
    MirSurfaceAttrib attrib,
    int value)
{
    return surface->configure(attrib, value);
}

void QtMirWindowManager::modify_surface(const std::shared_ptr<mir::scene::Session>&, const std::shared_ptr<mir::scene::Surface>&, const mir::shell::SurfaceSpecification&)
{
}
