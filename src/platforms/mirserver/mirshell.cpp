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
#include <mir/shell/display_layout.h>

namespace ms = mir::scene;
using mir::shell::AbstractShell;

MirShell::MirShell(
    std::shared_ptr<mir::shell::InputTargeter> const& input_targeter,
    std::shared_ptr<mir::scene::SurfaceCoordinator> const& surface_coordinator,
    std::shared_ptr<mir::scene::SessionCoordinator> const& session_coordinator,
    std::shared_ptr<mir::scene::PromptSessionManager> const& prompt_session_manager,
    std::shared_ptr<mir::shell::DisplayLayout> const& display_layout) :
    AbstractShell(input_targeter, surface_coordinator, session_coordinator, prompt_session_manager),
    m_displayLayout{display_layout}
{
    qCDebug(QTMIR_MIR_MESSAGES) << "MirShell::MirShell";
}

mir::frontend::SurfaceId MirShell::create_surface(std::shared_ptr<ms::Session> const& session, ms::SurfaceCreationParameters const& requestParameters)
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

     qCDebug(QTMIR_MIR_MESSAGES) << "MirShell::create_surface(): size requested ("
         << requestParameters.size.width.as_int() << "," << requestParameters.size.height.as_int() << ") and placed ("
         << placedParameters.size.width.as_int() << "," << placedParameters.size.height.as_int() << ")";

     tracepoint(qtmirserver, surfacePlacementEnd);

     return AbstractShell::create_surface(session, placedParameters);
}

int MirShell::set_surface_attribute(
    std::shared_ptr<mir::scene::Session> const& session,
    std::shared_ptr<mir::scene::Surface> const& surface,
    MirSurfaceAttrib attrib,
    int value)
{
    auto const result = AbstractShell::set_surface_attribute(session, surface, attrib, value);
    Q_EMIT surfaceAttributeChanged(surface.get(), attrib, result);

    return result;
}
