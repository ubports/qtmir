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

#ifndef QTMIR_WINDOWMANAGEMENTPOLICY_H
#define QTMIR_WINDOWMANAGEMENTPOLICY_H

// miral
#include "miral/canonical_window_manager.h"
#include <miral/set_window_managment_policy.h>

// Unity API
#include <unity/shell/application/Mir.h>

using namespace mir::geometry;

class QMirServer;

namespace qtmir {

class WindowModelNotifier;
class AppNotifier;
class WindowManagementPolicyPrivate;

class WindowManagementPolicy : public ::miral::CanonicalWindowManagerPolicy
{
public:

    // From miral::WindowManagementPolicy
    auto place_new_surface(const ::miral::ApplicationInfo &app_info,
                           const ::miral::WindowSpecification &request_parameters)
        -> ::miral::WindowSpecification override;

    void handle_window_ready(::miral::WindowInfo &windowInfo) override;
    void handle_modify_window(
           :: miral::WindowInfo &windowInfo,
            const ::miral::WindowSpecification &modifications) override;
    void handle_raise_window(::miral::WindowInfo &windowInfo) override;

    bool handle_keyboard_event(const MirKeyboardEvent *event) override;
    bool handle_touch_event(const MirTouchEvent *event) override;
    bool handle_pointer_event(const MirPointerEvent *event) override;

    void advise_begin() override;
    void advise_end() override;

    void advise_new_app(::miral::ApplicationInfo &application) override;
    void advise_delete_app(const ::miral::ApplicationInfo &application) override;

    void advise_new_window(const ::miral::WindowInfo &windowInfo) override;
    void advise_focus_lost(const ::miral::WindowInfo &info) override;
    void advise_focus_gained(const ::miral::WindowInfo &info) override;
    void advise_state_change(const ::miral::WindowInfo &info, MirSurfaceState state) override;
    void advise_move_to(const:: miral::WindowInfo &windowInfo, Point topLeft) override;
    void advise_resize(const ::miral::WindowInfo &info, const Size &newSize) override;
    void advise_delete_window(const ::miral::WindowInfo &windowInfo) override;
    void advise_raise(const std::vector<::miral::Window> &windows) override;

    // Methods for consumption by WindowControllerInterface
    virtual void deliver_keyboard_event(const MirKeyboardEvent *event, const ::miral::Window &window);
    virtual void deliver_touch_event   (const MirTouchEvent *event,    const ::miral::Window &window);
    virtual void deliver_pointer_event (const MirPointerEvent *event,  const ::miral::Window &window);

    virtual void activate(const ::miral::Window &window);
    virtual void resize(const ::miral::Window &window, const Size size);
    virtual void move  (const ::miral::Window &window, const Point topLeft);
    virtual void raise(const ::miral::Window &window);
    virtual void requestState(const ::miral::Window &window, const Mir::State state);

    virtual void ask_client_to_close(const ::miral::Window &window);
    virtual void forceClose(const ::miral::Window &window);

    qtmir::WindowModelNotifier& windowNotifier() const;
    qtmir::AppNotifier& appNotifier() const;

protected:
    WindowManagementPolicy(const ::miral::WindowManagerTools &tools, qtmir::WindowManagementPolicyPrivate& dd);
    std::shared_ptr<WindowManagementPolicyPrivate> d;
};


typedef std::function<std::shared_ptr<WindowManagementPolicy>(const ::miral::WindowManagerTools &tools, qtmir::WindowManagementPolicyPrivate& dd)> WindowManagmentPolicyCreator;

class BasicSetWindowManagementPolicy
{
public:
    explicit BasicSetWindowManagementPolicy(WindowManagmentPolicyCreator const& builder);
    ~BasicSetWindowManagementPolicy() = default;

    void operator()(QMirServer& server);
    WindowManagmentPolicyCreator builder() const;

private:
    struct Private;
    std::shared_ptr<Private> d;
};

template<typename Policy>
class SetWindowManagementPolicy : public BasicSetWindowManagementPolicy
{
public:
    template<typename ...Args>
    explicit SetWindowManagementPolicy(Args const& ...args) :
        BasicSetWindowManagementPolicy{[&args...](const ::miral::WindowManagerTools &tools, qtmir::WindowManagementPolicyPrivate& dd) {
            return std::make_shared<Policy>(tools, dd, args...);
    }} {}
};

} // namespace qtmir

#endif // QTMIR_WINDOWMANAGEMENTPOLICY_H
