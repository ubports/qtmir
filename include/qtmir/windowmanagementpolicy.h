/*
 * Copyright (C) 2017 Canonical, Ltd.
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
#include <miral/version.h>
#if MIRAL_VERSION > MIR_VERSION_NUMBER(1,3,1)
#include <miral/set_window_management_policy.h>
#else
#include <miral/set_window_managment_policy.h>
#endif

// Unity API
#include <unity/shell/application/Mir.h>

using namespace mir::geometry;

class QMirServer;
class QMargins;

namespace qtmir {

class WindowModelNotifier;
class AppNotifier;
class WindowManagementPolicyPrivate;

/*
    Provides window management callbacks for window behaviour customization
 */
class WindowManagementPolicy : public miral::CanonicalWindowManagerPolicy
{
public:

    // From miral::WindowManagementPolicy
    auto place_new_window(const miral::ApplicationInfo &app_info,
                           const miral::WindowSpecification &request_parameters)
        -> miral::WindowSpecification override;

    void handle_window_ready(miral::WindowInfo &windowInfo) override;
    void handle_modify_window(
           :: miral::WindowInfo &windowInfo,
            const miral::WindowSpecification &modifications) override;
    void handle_raise_window(miral::WindowInfo &windowInfo) override;

    auto confirm_placement_on_display(
        const miral::WindowInfo &window_info,
        MirWindowState new_state,
        const mir::geometry::Rectangle &new_placement) -> mir::geometry::Rectangle override;

    bool handle_keyboard_event(const MirKeyboardEvent *event) override;
    bool handle_touch_event(const MirTouchEvent *event) override;
    bool handle_pointer_event(const MirPointerEvent *event) override;

    void advise_begin() override;
    void advise_end() override;

    void advise_new_app(miral::ApplicationInfo &application) override;
    void advise_delete_app(const miral::ApplicationInfo &application) override;

    void advise_new_window(const miral::WindowInfo &windowInfo) override;
    void advise_focus_lost(const miral::WindowInfo &info) override;
    void advise_focus_gained(const miral::WindowInfo &info) override;
    void advise_state_change(const miral::WindowInfo &info, MirWindowState state) override;
    void advise_move_to(const:: miral::WindowInfo &windowInfo, Point topLeft) override;
    void advise_resize(const miral::WindowInfo &info, const Size &newSize) override;
    void advise_delete_window(const miral::WindowInfo &windowInfo) override;
    void advise_raise(const std::vector<miral::Window> &windows) override;

    void handle_request_drag_and_drop(miral::WindowInfo &window_info) override;
    void handle_request_move(miral::WindowInfo &window_info, const MirInputEvent *input_event) override;
    void handle_request_resize(miral::WindowInfo &window_info, const MirInputEvent *input_event, MirResizeEdge edge) override;

    Rectangle confirm_inherited_move(miral::WindowInfo const& windowInfo, Displacement movement) override;

protected:
    WindowManagementPolicy(const miral::WindowManagerTools &tools, std::shared_ptr<qtmir::WindowManagementPolicyPrivate> dd);

    std::shared_ptr<WindowManagementPolicyPrivate> d;
};

using WindowManagmentPolicyBuilder =
    std::function<std::shared_ptr<WindowManagementPolicy>(const miral::WindowManagerTools &tools, std::shared_ptr<qtmir::WindowManagementPolicyPrivate> dd)>;

class BasicSetWindowManagementPolicy
{
public:
    explicit BasicSetWindowManagementPolicy(WindowManagmentPolicyBuilder const& builder);
    ~BasicSetWindowManagementPolicy() = default;

    void operator()(QMirServer& server);
    WindowManagmentPolicyBuilder builder() const;

private:
    struct Private;
    std::shared_ptr<Private> d;
};

/*
    Set the window management policy to allow server customization

    usage:
    class MyWindowManagementPolicy : publi qtmir::WindowManagementPolicy
    {
    ...
    }

    qtmir::GuiServerApplication app(argc, argv, { SetWindowManagementPolicy<MyWindowManagementPolicy>() });
 */
template<typename Policy>
class SetWindowManagementPolicy : public BasicSetWindowManagementPolicy
{
public:
    template<typename ...Args>
    explicit SetWindowManagementPolicy(Args const& ...args) :
        BasicSetWindowManagementPolicy{[&args...](const miral::WindowManagerTools &tools, std::shared_ptr<qtmir::WindowManagementPolicyPrivate> dd) {
            return std::make_shared<Policy>(tools, dd, args...);
    }} {}
};

} // namespace qtmir

#endif // QTMIR_WINDOWMANAGEMENTPOLICY_H
