/*
 * Copyright (C) 2016-2017 Canonical, Ltd.
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

#include "windowmanagementpolicy.h"

#include "eventdispatch.h"
#include "initialsurfacesizes.h"
#include "screensmodel.h"
#include "surfaceobserver.h"

#include "miral/window_manager_tools.h"
#include "miral/window_specification.h"

#include "mirqtconversion.h"
#include "tracepoints.h"

namespace qtmir {
    std::shared_ptr<ExtraWindowInfo> getExtraInfo(const miral::WindowInfo &windowInfo) {
        return std::static_pointer_cast<ExtraWindowInfo>(windowInfo.userdata());
    }
}

using namespace qtmir;

WindowManagementPolicy::WindowManagementPolicy(const miral::WindowManagerTools &tools,
                                               qtmir::WindowModelNotifier &windowModel,
                                               qtmir::WindowController &windowController,
                                               qtmir::AppNotifier &appNotifier,
                                               const QSharedPointer<ScreensModel> screensModel)
    : CanonicalWindowManagerPolicy(tools)
    , m_windowModel(windowModel)
    , m_appNotifier(appNotifier)
    , m_screensModel(screensModel)
{
    qRegisterMetaType<qtmir::NewWindow>();
    qRegisterMetaType<std::vector<miral::Window>>();
    qRegisterMetaType<miral::ApplicationInfo>();
    windowController.setPolicy(this);
}

/* Following are hooks to allow custom policy be imposed */
miral::WindowSpecification WindowManagementPolicy::place_new_window(
    const miral::ApplicationInfo &appInfo,
    const miral::WindowSpecification &requestParameters)
{
    auto parameters = CanonicalWindowManagerPolicy::place_new_window(appInfo, requestParameters);

    if (!requestParameters.parent().is_set() || requestParameters.parent().value().lock().get() == nullptr) {

        int surfaceType = requestParameters.type().is_set() ? requestParameters.type().value() : -1;

        QSize initialSize = InitialSurfaceSizes::get(miral::pid_of(appInfo.application()));

        if (initialSize.isValid() && surfaceType == mir_window_type_normal) {
            parameters.size() = toMirSize(initialSize);
        }

        auto surfaceName = requestParameters.name().is_set() ? requestParameters.name().value() : "";

        if (surfaceName == "maliit-server") {
            parameters.type() = mir_window_type_inputmethod;
        }
    }

    parameters.userdata() = std::make_shared<ExtraWindowInfo>();

    return parameters;
}

void WindowManagementPolicy::handle_window_ready(miral::WindowInfo &windowInfo)
{
    Q_EMIT m_windowModel.windowReady(windowInfo);

    auto appInfo = tools.info_for(windowInfo.window().application());
    Q_EMIT m_appNotifier.appCreatedWindow(appInfo);
}

void WindowManagementPolicy::handle_modify_window(
    miral::WindowInfo &windowInfo,
    const miral::WindowSpecification &modificationsClient)
{
    miral::WindowSpecification modifications(modificationsClient);

    if (modifications.size().is_set()) {
        auto extraWindowInfo = getExtraInfo(windowInfo);
        QMutexLocker locker(&extraWindowInfo->mutex);
        if (!extraWindowInfo->allowClientResize) {
            modifications.size().consume();
        }
    }

    CanonicalWindowManagerPolicy::handle_modify_window(windowInfo, modifications);

    // TODO Once Qt processes the request we probably don't want to notify from here
    std::shared_ptr<mir::scene::Surface> surface{windowInfo.window()};
    if (SurfaceObserver *observer = SurfaceObserver::observerForSurface(surface.get())) {
        observer->notifySurfaceModifications(modifications);
    }
}

void WindowManagementPolicy::handle_raise_window(miral::WindowInfo &windowInfo)
{
    Q_EMIT m_windowModel.windowRequestedRaise(windowInfo);
}

Rectangle WindowManagementPolicy::confirm_placement_on_display(const miral::WindowInfo &/*window_info*/,
                                                               MirWindowState /*new_state*/,
                                                               const Rectangle &new_placement)
{
    return new_placement;
}

/* Handle input events - here just inject them into Qt event loop for later processing */
bool WindowManagementPolicy::handle_keyboard_event(const MirKeyboardEvent *event)
{
    m_eventFeeder.dispatchKey(event);
    return true;
}

bool WindowManagementPolicy::handle_touch_event(const MirTouchEvent *event)
{
    m_eventFeeder.dispatchTouch(event);
    return true;
}

bool WindowManagementPolicy::handle_pointer_event(const MirPointerEvent *event)
{
    m_eventFeeder.dispatchPointer(event);
    return true;
}

void WindowManagementPolicy::advise_new_window(const miral::WindowInfo &windowInfo)
{
    // TODO: attach surface observer here

    getExtraInfo(windowInfo)->persistentId = QString::fromStdString(tools.id_for_window(windowInfo.window()));

    // FIXME: remove when possible
    getExtraInfo(windowInfo)->state = toQtState(windowInfo.state());

    Q_EMIT m_windowModel.windowAdded(NewWindow{windowInfo});
}

void WindowManagementPolicy::advise_delete_window(const miral::WindowInfo &windowInfo)
{
    Q_EMIT m_windowModel.windowRemoved(windowInfo);
}

void WindowManagementPolicy::advise_raise(const std::vector<miral::Window> &windows)
{
    Q_EMIT m_windowModel.windowsRaised(windows);
}

void WindowManagementPolicy::advise_new_app(miral::ApplicationInfo &application)
{
    tracepoint(qtmirserver, starting);
    Q_EMIT m_appNotifier.appAdded(application);
}

void WindowManagementPolicy::advise_delete_app(const miral::ApplicationInfo &application)
{
    tracepoint(qtmirserver, stopping);
    Q_EMIT m_appNotifier.appRemoved(application);
}

void WindowManagementPolicy::advise_state_change(const miral::WindowInfo &windowInfo, MirWindowState state)
{
    auto extraWinInfo = getExtraInfo(windowInfo);

    // FIXME: Remove this mess once MirWindowState matches Mir::State
    if (state == mir_window_state_restored && extraWinInfo->state != Mir::RestoredState
            && toMirState(extraWinInfo->state) == state) {
        // Ignore. That MirWindowState is just a placeholder for a Mir::State value that has no counterpart
        // in MirWindowState.
    } else {
        extraWinInfo->state = toQtState(state);
    }

    Q_EMIT m_windowModel.windowStateChanged(windowInfo, extraWinInfo->state);
}

void WindowManagementPolicy::advise_move_to(const miral::WindowInfo &windowInfo, Point topLeft)
{
    Q_EMIT m_windowModel.windowMoved(windowInfo, toQPoint(topLeft));
}

void WindowManagementPolicy::advise_resize(const miral::WindowInfo &windowInfo, const Size &newSize)
{
    Q_EMIT m_windowModel.windowResized(windowInfo, toQSize(newSize));
}

void WindowManagementPolicy::advise_focus_lost(const miral::WindowInfo &windowInfo)
{
    Q_EMIT m_windowModel.windowFocusChanged(windowInfo, false);
}

void WindowManagementPolicy::advise_focus_gained(const miral::WindowInfo &windowInfo)
{
    // update Qt model ASAP, before applying Mir policy
    Q_EMIT m_windowModel.windowFocusChanged(windowInfo, true);

    CanonicalWindowManagerPolicy::advise_focus_gained(windowInfo);
}

void WindowManagementPolicy::advise_begin()
{
    Q_EMIT m_windowModel.modificationsStarted();
}

void WindowManagementPolicy::advise_end()
{
    Q_EMIT m_windowModel.modificationsEnded();
}

void WindowManagementPolicy::advise_output_create(miral::Output const& output)
{
    Q_UNUSED(output);
    m_screensModel->update();
}

void WindowManagementPolicy::advise_output_update(miral::Output const& updated, miral::Output const& original)
{
    Q_UNUSED(updated);
    Q_UNUSED(original);
    m_screensModel->update();
}

void WindowManagementPolicy::advise_output_delete(miral::Output const& output)
{
    Q_UNUSED(output);
    m_screensModel->update();
}

void WindowManagementPolicy::ensureWindowIsActive(const miral::Window &window)
{
    tools.invoke_under_lock([&window, this]() {
        if (tools.active_window() != window) {
            tools.select_active_window(window);
        }
    });
}

QRect WindowManagementPolicy::getConfinementRect(const QRect rect) const
{
    QRect confinementRect;
    for (const QRect r : m_confinementRegions) {
        if (r.intersects(rect)) {
            confinementRect = r;
            // TODO: What if there are multiple confinement regions and they intersect??
            break;
        }
    }
    return confinementRect;
}

/* Following methods all called from the Qt GUI thread to deliver events to clients */
void WindowManagementPolicy::deliver_keyboard_event(const MirKeyboardEvent *event,
                                                    const miral::Window &window)
{
    if (mir_keyboard_event_action(event) == mir_keyboard_action_down) {
        ensureWindowIsActive(window);
    }

    dispatchInputEvent(window, mir_keyboard_event_input_event(event));
}

void WindowManagementPolicy::deliver_touch_event(const MirTouchEvent *event,
                                                 const miral::Window &window)
{
    ensureWindowIsActive(window);

    dispatchInputEvent(window, mir_touch_event_input_event(event));
}

void WindowManagementPolicy::deliver_pointer_event(const MirPointerEvent *event,
                                                   const miral::Window &window)
{
    // Prevent mouse hover events causing window focus to change
    if (mir_pointer_event_action(event) == mir_pointer_action_button_down) {
        ensureWindowIsActive(window);
    }

    dispatchInputEvent(window, mir_pointer_event_input_event(event));
}

/* Methods to allow Shell to request changes to the window stack. Called from the Qt GUI thread */

// raises the window tree and focus it.
void WindowManagementPolicy::activate(const miral::Window &window)
{
    try {
        if (window) {
            auto &windowInfo = tools.info_for(window);

            // restore from minimized if needed
            if (windowInfo.state() == mir_window_state_minimized) {
                auto extraInfo = getExtraInfo(windowInfo);
                Q_ASSERT(extraInfo->previousState != Mir::MinimizedState);
                requestState(window, extraInfo->previousState);
            }
        }

        tools.invoke_under_lock([&]() {
            tools.select_active_window(window);
        });
    } catch (const std::out_of_range&) {
        // usually shell trying to operate on a window which already closed, just ignore
        // (throws from tools.info_for(...) ususally)
        // TODO: Same issue as resize below
    }
}

// raises the window tree
void WindowManagementPolicy::raise(const miral::Window &window)
{
    tools.invoke_under_lock([&window, this]() {
        tools.raise_tree(window);
    });
}

void WindowManagementPolicy::resize(const miral::Window &window, const Size size)
{
    miral::WindowSpecification modifications;
    modifications.size() = size;
    tools.invoke_under_lock([&window, &modifications, this]() {
        try {
            tools.modify_window(tools.info_for(window), modifications);
        } catch (const std::out_of_range&) {
            // usually shell trying to operate on a window which already closed, just ignore
            // TODO: MirSurface extends the miral::Window lifetime by holding a shared pointer to
            // the mir::scene::Surface, meaning it cannot detect when the window has been closed
            // and thus avoid calling this method.
        }
    });
}

void WindowManagementPolicy::move(const miral::Window &window, const Point topLeft)
{
    miral::WindowSpecification modifications;
    modifications.top_left() = topLeft;
    tools.invoke_under_lock([&window, &modifications, this]() {
        try {
            tools.modify_window(tools.info_for(window), modifications);
        } catch (const std::out_of_range&) {
            // usually shell trying to operate on a window which already closed, just ignore
            // TODO: see above comment in resize, same issue
        }
    });
}

void WindowManagementPolicy::ask_client_to_close(const miral::Window &window)
{
    tools.invoke_under_lock([&window, this]() {
        tools.ask_client_to_close(window);
    });
}

void WindowManagementPolicy::forceClose(const miral::Window &window)
{
    tools.invoke_under_lock([&window, this]() {
        tools.force_close(window);
    });
}

void WindowManagementPolicy::set_window_confinement_regions(const QVector<QRect> &regions)
{
    m_confinementRegions = regions;

    // TODO: update window positions to respect new boundary.
}

void WindowManagementPolicy::set_window_margins(MirWindowType windowType, const QMargins &margins)
{
    m_windowMargins[windowType] = margins;

    // TODO: update window positions/sizes to respect new margins.
}

void WindowManagementPolicy::requestState(const miral::Window &window, const Mir::State state)
{
    auto &windowInfo = tools.info_for(window);
    auto extraWinInfo = getExtraInfo(windowInfo);

    if (extraWinInfo->state == state)
        return;

    miral::WindowSpecification modifications;
    modifications.state() = toMirState(state);

    // TODO: What if the window modification fails? Is that possible?
    //       Assuming here that the state will indeed change
    extraWinInfo->previousState = extraWinInfo->state;
    extraWinInfo->state = state;

    if (modifications.state() == windowInfo.state()) {
        Q_EMIT m_windowModel.windowStateChanged(windowInfo, state);
    } else {
        tools.invoke_under_lock([&]() {
            tools.modify_window(windowInfo, modifications);
        });
    }
}

Rectangle WindowManagementPolicy::confirm_inherited_move(miral::WindowInfo const& windowInfo, Displacement movement)
{
    if (m_confinementRegions.isEmpty()) {
        return CanonicalWindowManagerPolicy::confirm_inherited_move(windowInfo, movement);
    }

    auto window = windowInfo.window();
    const QMargins windowMargins = m_windowMargins[windowInfo.type()];
    QRect windowGeom(toQPoint(window.top_left()), toQSize(window.size()));

    QRect geom = windowGeom.marginsAdded(windowMargins);
    const QRect confinementRect = getConfinementRect(geom);

    int x = geom.x();
    int y = geom.y();
    int moveX = movement.dx.as_int();
    int moveY = movement.dy.as_int();

    // If the child window is already partially beyond the available desktop area (most likely because the user
    // explicitly moved it there) we won't pull it back, unless the inherited movement is this direction, but also won't
    // push it even further astray. But if it currently is completely within the available desktop area boundaries
    // we won't let go beyond it.

    if (moveX > 0) {
        if (geom.right() < confinementRect.right()) {
            x = qMin(x + moveX, confinementRect.right() + 1 - geom.width()); // +1 because QRect::right() weird
        }
    } else {
        if (geom.x() > confinementRect.left()) {
            x = qMax(x + moveX, confinementRect.left());
        }
    }

    if (moveY > 0) {
        if (geom.bottom() < confinementRect.bottom()) {
            y = qMin(y + moveY, confinementRect.bottom() + 1 - geom.height()); // +1 because QRect::bottom() weird
        }
    } else {
        if (geom.y() > confinementRect.top()) {
            y = qMax(y + moveY, confinementRect.top());
        }
    }

    geom.moveTo(x, y);
    return toMirRectangle(geom.marginsRemoved(windowMargins));
}

void WindowManagementPolicy::handle_request_drag_and_drop(miral::WindowInfo &/*window_info*/)
{

}

void WindowManagementPolicy::handle_request_move(miral::WindowInfo &/*window_info*/,
                                                 const MirInputEvent */*input_event*/)
{

}

void WindowManagementPolicy::handle_request_resize(miral::WindowInfo &/*window_info*/,
                                                   const MirInputEvent */*input_event*/, MirResizeEdge /*edge*/)
{

}
