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

namespace
{
class MirWindowManagerImpl : public MirWindowManager
{
public:

    MirWindowManagerImpl(const std::shared_ptr<mir::shell::DisplayLayout> &displayLayout);

    void add_session(std::shared_ptr<mir::scene::Session> const& session) override;

    void remove_session(std::shared_ptr<mir::scene::Session> const& session) override;

    mir::frontend::SurfaceId add_surface(
        std::shared_ptr<mir::scene::Session> const& session,
        mir::scene::SurfaceCreationParameters const& params,
        std::function<mir::frontend::SurfaceId(std::shared_ptr<mir::scene::Session> const& session, mir::scene::SurfaceCreationParameters const& params)> const& build) override;

    void remove_surface(
        std::shared_ptr<mir::scene::Session> const& session,
        std::weak_ptr<mir::scene::Surface> const& surface) override;

    void add_display(mir::geometry::Rectangle const& area) override;

    void remove_display(mir::geometry::Rectangle const& area) override;

    bool handle_keyboard_event(MirKeyboardEvent const* event) override;

    bool handle_touch_event(MirTouchEvent const* event) override;

    bool handle_pointer_event(MirPointerEvent const* event) override;

    int set_surface_attribute(
        std::shared_ptr<mir::scene::Session> const& session,
        std::shared_ptr<mir::scene::Surface> const& surface,
        MirSurfaceAttrib attrib,
        int value) override;

    void handle_raise_surface(
        std::shared_ptr<mir::scene::Session> const& session,
        std::shared_ptr<mir::scene::Surface> const& surface,
        uint64_t timestamp) override;

    void modify_surface(
        const std::shared_ptr<mir::scene::Session>&,
        const std::shared_ptr<mir::scene::Surface>& surface,
        const mir::shell::SurfaceSpecification& modifications) override;

private:
    std::shared_ptr<mir::shell::DisplayLayout> const m_displayLayout;
};

}

MirWindowManagerImpl::MirWindowManagerImpl(const std::shared_ptr<mir::shell::DisplayLayout> &displayLayout) :
    m_displayLayout{displayLayout}
{
    qCDebug(QTMIR_MIR_MESSAGES) << "MirWindowManagerImpl::MirWindowManagerImpl";
}

void MirWindowManagerImpl::add_session(std::shared_ptr<ms::Session> const& /*session*/)
{
}

void MirWindowManagerImpl::remove_session(std::shared_ptr<ms::Session> const& /*session*/)
{
}

mir::frontend::SurfaceId MirWindowManagerImpl::add_surface(
    std::shared_ptr<ms::Session> const& session,
    ms::SurfaceCreationParameters const& requestParameters,
    std::function<mir::frontend::SurfaceId(std::shared_ptr<ms::Session> const& session, ms::SurfaceCreationParameters const& params)> const& build)
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

    qCDebug(QTMIR_MIR_MESSAGES) << "MirWindowManagerImpl::add_surface(): size requested ("
                                << requestParameters.size.width.as_int() << "," << requestParameters.size.height.as_int() << ") and placed ("
                                << placedParameters.size.width.as_int() << "," << placedParameters.size.height.as_int() << ")";

    tracepoint(qtmirserver, surfacePlacementEnd);

    return build(session, placedParameters);
}

void MirWindowManagerImpl::remove_surface(
    std::shared_ptr<ms::Session> const& /*session*/,
    std::weak_ptr<ms::Surface> const& /*surface*/)
{
}

void MirWindowManagerImpl::add_display(mir::geometry::Rectangle const& /*area*/)
{
}

void MirWindowManagerImpl::remove_display(mir::geometry::Rectangle const& /*area*/)
{
}

bool MirWindowManagerImpl::handle_keyboard_event(MirKeyboardEvent const* /*event*/)
{
    return false;
}

bool MirWindowManagerImpl::handle_touch_event(MirTouchEvent const* /*event*/)
{
    return false;
}

bool MirWindowManagerImpl::handle_pointer_event(MirPointerEvent const* /*event*/)
{
    return false;
}

void handle_raise_surface(
    std::shared_ptr<mir::scene::Session> const& /*session*/,
    std::shared_ptr<mir::scene::Surface> const& /*surface*/,
    uint64_t /*timestamp*/)
{
}

int MirWindowManagerImpl::set_surface_attribute(
    std::shared_ptr<ms::Session> const& /*session*/,
    std::shared_ptr<ms::Surface> const& surface,
    MirSurfaceAttrib attrib,
    int value)
{
    return surface->configure(attrib, value);
}

void MirWindowManagerImpl::modify_surface(const std::shared_ptr<mir::scene::Session>&,
                                          const std::shared_ptr<mir::scene::Surface>& surface,
                                          const mir::shell::SurfaceSpecification& modifications)
{
    if (modifications.name.is_set()) {
        surface->rename(modifications.name.value());
    }
}

std::unique_ptr<MirWindowManager> MirWindowManager::create(
    mir::shell::FocusController* /*focus_controller*/,
    const std::shared_ptr<mir::shell::DisplayLayout> &displayLayout)
{
    return std::make_unique<MirWindowManagerImpl>(displayLayout);
}
