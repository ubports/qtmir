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

#ifndef QTMIR_DISPLAYCONFIGURATIONPOLICY_H
#define QTMIR_DISPLAYCONFIGURATIONPOLICY_H

//qtmir
#include "qtmir/miral/display_configuration_policy.h"

// mir
#include <mir/server.h>

#include <memory>

class QMirServer;

namespace qtmir
{

/*
    Provides callback for display configuration customization.

    This default policy applies scaling and form factor decisions on the configuration.
    It can be subclassed/overriden by using the SetDisplayConfigurationPolicy<> initializer.

    usage:
    struct MyDisplayConfigurationPolicy : qtmir::DisplayConfigurationPolicy
    {
        void apply_to(mir::graphics::DisplayConfiguration& conf) override;
    }

    qtmir::GuiServerApplication app(argc, argv, { SetDisplayConfigurationPolicy<MyDisplayConfigurationPolicy>() });
 */
class DisplayConfigurationPolicy : public miral::experimental::DisplayConfigurationPolicy
{
public:
    DisplayConfigurationPolicy();

    virtual void apply_to(mir::graphics::DisplayConfiguration& conf) override;

private:
    struct Private;
    std::shared_ptr<Private> d;
};

using DisplayConfigurationPolicyWrapper = std::function<std::shared_ptr<miral::experimental::DisplayConfigurationPolicy>()>;

/*
    Base class for access to set the display configuration policy
 */
class BasicSetDisplayConfigurationPolicy
{
public:
    explicit BasicSetDisplayConfigurationPolicy(DisplayConfigurationPolicyWrapper const& builder);
    ~BasicSetDisplayConfigurationPolicy() = default;

    void operator()(QMirServer& server);

private:
    struct Self;
    std::shared_ptr<Self> self;
};

/*
    Set the display configuration policy to allow server customization
 */
template<typename Policy>
class SetDisplayConfigurationPolicy : public BasicSetDisplayConfigurationPolicy
{
public:
    template<typename ...Args>
    explicit SetDisplayConfigurationPolicy(Args const& ...args) :
        BasicSetDisplayConfigurationPolicy{
            [&args...]() { return std::make_shared<Policy>(args...); }} {}
};

} // namespace qtmir

#endif // QTMIR_DISPLAYCONFIGURATIONPOLICY_H
