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

#include "wrappedwindowmanagementpolicy.h"

#include "screensmodel.h"
#include "surfaceobserver.h"

#include "miral/window_manager_tools.h"
#include "miral/window_specification.h"

#include "mirqtconversion.h"
#include "qmirserver.h"

#include <mir/scene/surface.h>
#include <QDebug>

namespace qtmir {
    std::shared_ptr<ExtraWindowInfo> getExtraInfo(const miral::WindowInfo &windowInfo) {
        return std::static_pointer_cast<ExtraWindowInfo>(windowInfo.userdata());
    }

    struct BasicSetWindowManagementPolicy::Private
    {
        Private(WindowManagmentPolicyCreator const& builder) :
            builder{builder} {}

        WindowManagmentPolicyCreator builder;
    };

    BasicSetWindowManagementPolicy::BasicSetWindowManagementPolicy(WindowManagmentPolicyCreator const& builder)
        : d(new BasicSetWindowManagementPolicy::Private(builder))
    {
    }

    void BasicSetWindowManagementPolicy::operator()(QMirServer &server)
    {
        server.overrideWindowManagementPolicy(*this);
    }

    WindowManagmentPolicyCreator BasicSetWindowManagementPolicy::builder() const
    {
        return d->builder;
    }

    struct WindowManagementPolicyPrivate
    {
        WindowManagementPolicyPrivate(qtmir::WindowModelNotifier &windowModel,
                                      qtmir::AppNotifier &appNotifier,
                                      const std::shared_ptr<QtEventFeeder>& eventFeeder)
            : m_windowModel(windowModel)
            , m_appNotifier(appNotifier)
            , m_eventFeeder(eventFeeder)
        {}

        qtmir::WindowModelNotifier &m_windowModel;
        qtmir::AppNotifier &m_appNotifier;
        const std::shared_ptr<QtEventFeeder> m_eventFeeder;
    };

    WindowManagementPolicy::WindowManagementPolicy(const miral::WindowManagerTools &tools, qtmir::WindowManagementPolicyPrivate& dd)
        : miral::CanonicalWindowManagerPolicy(tools)
        , d(&dd)
    {
    }

    miral::WindowSpecification WindowManagementPolicy::place_new_surface(
        const miral::ApplicationInfo &app_info,
        const miral::WindowSpecification &request_parameters)
    {
        auto parameters = CanonicalWindowManagerPolicy::place_new_surface(app_info, request_parameters);

        parameters.userdata() = std::make_shared<ExtraWindowInfo>();

        return parameters;
    }

    void WindowManagementPolicy::handle_window_ready(miral::WindowInfo &windowInfo)
    {
        CanonicalWindowManagerPolicy::handle_window_ready(windowInfo);

        Q_EMIT d->m_windowModel.windowReady(windowInfo);

        auto appInfo = tools.info_for(windowInfo.window().application());
        Q_EMIT d->m_appNotifier.appCreatedWindow(appInfo);
    }

    void WindowManagementPolicy::handle_modify_window(miral::WindowInfo &windowInfo, const miral::WindowSpecification &modifications)
    {
        // TODO this applies the default policy. Qt needs to process the request instead
        CanonicalWindowManagerPolicy::handle_modify_window(windowInfo, modifications);

        // TODO Once Qt processes the request we probably don't want to notify from here
        std::shared_ptr<mir::scene::Surface> surface{windowInfo.window()};
        if (SurfaceObserver *observer = SurfaceObserver::observerForSurface(surface.get())) {
            observer->notifySurfaceModifications(modifications);
        }
    }

    void WindowManagementPolicy::handle_raise_window(miral::WindowInfo &windowInfo)
    {
        CanonicalWindowManagerPolicy::handle_raise_window(windowInfo);
    }

    bool WindowManagementPolicy::handle_keyboard_event(const MirKeyboardEvent *event)
    {
        d->m_eventFeeder->dispatchKey(event);
        return true;
    }

    bool WindowManagementPolicy::handle_touch_event(const MirTouchEvent *event)
    {
        d->m_eventFeeder->dispatchTouch(event);
        return true;
    }

    bool WindowManagementPolicy::handle_pointer_event(const MirPointerEvent *event)
    {
        d->m_eventFeeder->dispatchPointer(event);
        return true;
    }

    void WindowManagementPolicy::advise_begin()
    {
        Q_EMIT d->m_windowModel.modificationsStarted();
    }

    void WindowManagementPolicy::advise_end()
    {
        Q_EMIT d->m_windowModel.modificationsEnded();
    }

    void WindowManagementPolicy::advise_new_app(miral::ApplicationInfo &application)
    {
        Q_EMIT d->m_appNotifier.appAdded(application);
    }

    void WindowManagementPolicy::advise_delete_app(const miral::ApplicationInfo &application)
    {
        Q_EMIT d->m_appNotifier.appRemoved(application);
    }

    void WindowManagementPolicy::advise_new_window(const miral::WindowInfo &windowInfo)
    {
        // TODO: attach surface observer here

        getExtraInfo(windowInfo)->persistentId = QString::fromStdString(tools.id_for_window(windowInfo.window()));

        // FIXME: remove when possible
        getExtraInfo(windowInfo)->state = toQtState(windowInfo.state());

        Q_EMIT d->m_windowModel.windowAdded(NewWindow{windowInfo});
    }

    void WindowManagementPolicy::advise_focus_lost(const miral::WindowInfo &windowInfo)
    {
        Q_EMIT d->m_windowModel.windowFocusChanged(windowInfo, false);
    }

    void WindowManagementPolicy::advise_focus_gained(const miral::WindowInfo &windowInfo)
    {
        // update Qt model ASAP, before applying Mir policy
        Q_EMIT d->m_windowModel.windowFocusChanged(windowInfo, true);

        CanonicalWindowManagerPolicy::advise_focus_gained(windowInfo);
    }

    void WindowManagementPolicy::advise_state_change(const miral::WindowInfo &windowInfo, MirSurfaceState state)
    {
        auto extraWinInfo = getExtraInfo(windowInfo);

        // FIXME: Remove this mess once MirSurfaceState matches Mir::State
        if (state == mir_surface_state_restored && extraWinInfo->state != Mir::RestoredState
                && toMirState(extraWinInfo->state) == state) {
            // Ignore. That MirSurfaceState is just a placeholder for a Mir::State value that has no counterpart
            // in MirSurfaceState.
        } else {
            extraWinInfo->state = toQtState(state);
        }

        Q_EMIT d->m_windowModel.windowStateChanged(windowInfo, extraWinInfo->state);
    }

    void WindowManagementPolicy::advise_move_to(const miral::WindowInfo &windowInfo, Point topLeft)
    {
        Q_EMIT d->m_windowModel.windowMoved(windowInfo, toQPoint(topLeft));
    }

    void WindowManagementPolicy::advise_resize(const miral::WindowInfo &windowInfo, const Size &newSize)
    {
        Q_EMIT d->m_windowModel.windowResized(windowInfo, toQSize(newSize));
    }

    void WindowManagementPolicy::advise_delete_window(const miral::WindowInfo &windowInfo)
    {
        Q_EMIT d->m_windowModel.windowRemoved(windowInfo);
    }

    void WindowManagementPolicy::advise_raise(const std::vector<miral::Window> &windows)
    {
        Q_EMIT d->m_windowModel.windowsRaised(windows);
    }

    /* Following methods all called from the Qt GUI thread to deliver events to clients */
    void WindowManagementPolicy::deliver_keyboard_event(const MirKeyboardEvent *event,
                                                        const miral::Window &window)
    {
        tools.invoke_under_lock([&window, this]() {
           tools.select_active_window(window);
        });
        auto e = reinterpret_cast<MirEvent const*>(event); // naughty

        if (auto surface = std::weak_ptr<mir::scene::Surface>(window).lock()) {
            surface->consume(e);
        }
    }

    void WindowManagementPolicy::deliver_touch_event(const MirTouchEvent *event,
                                                     const miral::Window &window)
    {
        tools.invoke_under_lock([&window, this]() {
            tools.select_active_window(window);
        });
        auto e = reinterpret_cast<MirEvent const*>(event); // naughty

        if (auto surface = std::weak_ptr<mir::scene::Surface>(window).lock()) {
            surface->consume(e);
        }
    }

    void WindowManagementPolicy::deliver_pointer_event(const MirPointerEvent *event,
                                                       const miral::Window &window)
    {
        // Prevent mouse hover events causing window focus to change
        if (mir_pointer_event_action(event) == mir_pointer_action_button_down) {
            tools.invoke_under_lock([&window, this]() {
                tools.select_active_window(window);
            });
        }
        auto e = reinterpret_cast<MirEvent const*>(event); // naughty

        if (auto surface = std::weak_ptr<mir::scene::Surface>(window).lock()) {
            surface->consume(e);
        }
    }

    /* Methods to allow Shell to request changes to the window stack. Called from the Qt GUI thread */

    // raises the window tree and focus it.
    void WindowManagementPolicy::activate(const miral::Window &window)
    {
        if (window) {
            auto &windowInfo = tools.info_for(window);

            // restore from minimized if needed
            if (windowInfo.state() == mir_surface_state_minimized) {
                auto extraInfo = getExtraInfo(windowInfo);
                Q_ASSERT(extraInfo->previousState != Mir::MinimizedState);
                requestState(window, extraInfo->previousState);
            }
        }

        tools.invoke_under_lock([&]() {
            tools.select_active_window(window);
        });
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
            Q_EMIT d->m_windowModel.windowStateChanged(windowInfo, state);
        } else {
            tools.invoke_under_lock([&]() {
                tools.modify_window(windowInfo, modifications);
            });
        }
    }

    WindowModelNotifier &WindowManagementPolicy::windowNotifier() const
    {
        return d->m_windowModel;
    }

    AppNotifier &WindowManagementPolicy::appNotifier() const
    {
        return d->m_appNotifier;
    }
}

using namespace qtmir;

WrappedWindowManagementPolicy::WrappedWindowManagementPolicy(const miral::WindowManagerTools &tools,
                                               qtmir::WindowModelNotifier &windowModel,
                                               qtmir::WindowController &windowController,
                                               qtmir::AppNotifier &appNotifier,
                                               const QSharedPointer<ScreensModel> screensModel,
                                               const qtmir::WindowManagmentPolicyCreator& wmBuilder)
    : qtmir::WindowManagementPolicy(tools, *new qtmir::WindowManagementPolicyPrivate(windowModel,
                                                                                     appNotifier,
                                                                                     std::make_shared<QtEventFeeder>(screensModel)))
    , m_wrapper(wmBuilder(tools, *new qtmir::WindowManagementPolicyPrivate(d->m_windowModel,
                                                                           d->m_appNotifier,
                                                                           d->m_eventFeeder)))
{
    qRegisterMetaType<qtmir::NewWindow>();
    qRegisterMetaType<std::vector<miral::Window>>();
    qRegisterMetaType<miral::ApplicationInfo>();
    windowController.setPolicy(this);
}

/* Following are hooks to allow custom policy be imposed */
miral::WindowSpecification WrappedWindowManagementPolicy::place_new_surface(
    const miral::ApplicationInfo &app_info,
    const miral::WindowSpecification &request_parameters)
{
    return m_wrapper->place_new_surface(app_info, request_parameters);
}

void WrappedWindowManagementPolicy::handle_window_ready(miral::WindowInfo &windowInfo)
{
    m_wrapper->handle_window_ready(windowInfo);
}

void WrappedWindowManagementPolicy::handle_modify_window(
    miral::WindowInfo &windowInfo,
    const miral::WindowSpecification &modifications)
{
    m_wrapper->handle_modify_window(windowInfo, modifications);
}

void WrappedWindowManagementPolicy::handle_raise_window(miral::WindowInfo &windowInfo)
{
    m_wrapper->handle_raise_window(windowInfo);
}

/* Handle input events - here just inject them into Qt event loop for later processing */
bool WrappedWindowManagementPolicy::handle_keyboard_event(const MirKeyboardEvent *event)
{
    return m_wrapper->handle_keyboard_event(event);
}

bool WrappedWindowManagementPolicy::handle_touch_event(const MirTouchEvent *event)
{
    return m_wrapper->handle_touch_event(event);
}

bool WrappedWindowManagementPolicy::handle_pointer_event(const MirPointerEvent *event)
{
    return m_wrapper->handle_pointer_event(event);
}

void WrappedWindowManagementPolicy::advise_begin()
{
    m_wrapper->advise_begin();
}

void WrappedWindowManagementPolicy::advise_end()
{
    m_wrapper->advise_end();
}

void WrappedWindowManagementPolicy::advise_new_window(const miral::WindowInfo &windowInfo)
{
    m_wrapper->advise_new_window(windowInfo);
}

void WrappedWindowManagementPolicy::advise_delete_window(const miral::WindowInfo &windowInfo)
{
    m_wrapper->advise_delete_window(windowInfo);
}

void WrappedWindowManagementPolicy::advise_raise(const std::vector<miral::Window> &windows)
{
    m_wrapper->advise_raise(windows);
}

void WrappedWindowManagementPolicy::advise_new_app(miral::ApplicationInfo &application)
{
    m_wrapper->advise_new_app(application);
}

void WrappedWindowManagementPolicy::advise_delete_app(const miral::ApplicationInfo &application)
{
    m_wrapper->advise_delete_app(application);
}

void WrappedWindowManagementPolicy::advise_focus_lost(const miral::WindowInfo &info)
{
    m_wrapper->advise_focus_lost(info);
}

void WrappedWindowManagementPolicy::advise_focus_gained(const miral::WindowInfo &info)
{
    m_wrapper->advise_focus_gained(info);
}

void WrappedWindowManagementPolicy::advise_state_change(const miral::WindowInfo &info, MirSurfaceState state)
{
    m_wrapper->advise_state_change(info, state);
}

void WrappedWindowManagementPolicy::advise_move_to(const miral::WindowInfo &windowInfo, Point topLeft)
{
    m_wrapper->advise_move_to(windowInfo, topLeft);
}

void WrappedWindowManagementPolicy::advise_resize(const miral::WindowInfo &info, const Size &newSize)
{
    m_wrapper->advise_resize(info, newSize);
}

void WrappedWindowManagementPolicy::deliver_keyboard_event(const MirKeyboardEvent *event, const miral::Window &window)
{
    m_wrapper->deliver_keyboard_event(event, window);
}

void WrappedWindowManagementPolicy::deliver_touch_event(const MirTouchEvent *event, const miral::Window &window)
{
    m_wrapper->deliver_touch_event(event, window);
}

void WrappedWindowManagementPolicy::deliver_pointer_event(const MirPointerEvent *event, const miral::Window &window)
{
    m_wrapper->deliver_pointer_event(event, window);
}

void WrappedWindowManagementPolicy::activate(const miral::Window &window)
{
    m_wrapper->activate(window);
}

void WrappedWindowManagementPolicy::resize(const miral::Window &window, const Size size)
{
    m_wrapper->resize(window, size);
}

void WrappedWindowManagementPolicy::move(const miral::Window &window, const Point topLeft)
{
    m_wrapper->move(window, topLeft);
}

void WrappedWindowManagementPolicy::raise(const miral::Window &window)
{
    m_wrapper->raise(window);
}

void WrappedWindowManagementPolicy::requestState(const miral::Window &window, const Mir::State state)
{
    m_wrapper->requestState(window, state);
}

void WrappedWindowManagementPolicy::ask_client_to_close(const miral::Window &window)
{
    m_wrapper->ask_client_to_close(window);
}

void WrappedWindowManagementPolicy::forceClose(const miral::Window &window)
{
    m_wrapper->forceClose(window);
}
