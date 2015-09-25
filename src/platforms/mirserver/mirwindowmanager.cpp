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

#include "mirwindowmanager.h"
#include "logging.h"
#include "tracepoints.h" // generated from tracepoints.tp

#include <mir/geometry/rectangle.h>
#include <mir/scene/session.h>
#include <mir/scene/surface_creation_parameters.h>
#include <mir/scene/surface.h>
#include <mir/shell/display_layout.h>

namespace ms = mir::scene;

MirWindowManager::MirWindowManager(const std::shared_ptr<mir::shell::DisplayLayout> &displayLayout) :
    m_displayLayout{displayLayout}
{
    qCDebug(QTMIR_MIR_MESSAGES) << "MirWindowManager::MirWindowManager";
}

void MirWindowManager::add_session(std::shared_ptr<ms::Session> const& /*session*/)
{
}

void MirWindowManager::remove_session(std::shared_ptr<ms::Session> const& /*session*/)
{
}

auto MirWindowManager::add_surface(
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

    qCDebug(QTMIR_MIR_MESSAGES) << "MirWindowManager::add_surface(): size requested ("
                                << requestParameters.size.width.as_int() << "," << requestParameters.size.height.as_int() << ") and placed ("
                                << placedParameters.size.width.as_int() << "," << placedParameters.size.height.as_int() << ")";

    tracepoint(qtmirserver, surfacePlacementEnd);

    return build(session, placedParameters);
}

void MirWindowManager::remove_surface(
    std::shared_ptr<ms::Session> const& /*session*/,
    std::weak_ptr<ms::Surface> const& /*surface*/)
{
}

void MirWindowManager::add_display(mir::geometry::Rectangle const& /*area*/)
{
}

void MirWindowManager::remove_display(mir::geometry::Rectangle const& /*area*/)
{
}

bool MirWindowManager::handle_keyboard_event(MirKeyboardEvent const* /*event*/)
{
    return false;
}

bool MirWindowManager::handle_touch_event(MirTouchEvent const* /*event*/)
{
    return false;
}

bool MirWindowManager::handle_pointer_event(MirPointerEvent const* /*event*/)
{
    return false;
}

int MirWindowManager::set_surface_attribute(
    std::shared_ptr<ms::Session> const& /*session*/,
    std::shared_ptr<ms::Surface> const& surface,
    MirSurfaceAttrib attrib,
    int value)
{
    return surface->configure(attrib, value);
}

void MirWindowManager::modify_surface(const std::shared_ptr<mir::scene::Session>&,
                                      const std::shared_ptr<mir::scene::Surface>& surface,
                                      const mir::shell::SurfaceSpecification& modifications)
{
    qWarning() << "CAYBRO:" << Q_FUNC_INFO << "surface modified";
    if (modifications.name.is_set()) {
        surface->rename(modifications.name.value());
        qWarning() << "CAYBRO:" << Q_FUNC_INFO << "surface renamed to:" << QString::fromStdString(modifications.name.value());
    }
}
