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

namespace qtmir {

class DisplayConfigurationPolicy : public miral::DisplayConfigurationPolicy
{
public:
    DisplayConfigurationPolicy(std::shared_ptr<miral::DisplayConfigurationPolicy> const& wrapped);

    virtual void apply_to(mir::graphics::DisplayConfiguration& conf);

private:
    struct Private;
    std::shared_ptr<Private> d;
};


using DisplayConfigurationPolicyWrapper =
    std::function<std::shared_ptr<miral::DisplayConfigurationPolicy>(std::shared_ptr<miral::DisplayConfigurationPolicy> const&)>;

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

template<typename Policy>
class SetDisplayConfigurationPolicy : public BasicSetDisplayConfigurationPolicy
{
public:
    template<typename ...Args>
    explicit SetDisplayConfigurationPolicy(Args const& ...args) :
        BasicSetDisplayConfigurationPolicy{
            [&args...](std::shared_ptr<miral::DisplayConfigurationPolicy> const& wrapped) {
                return std::make_shared<Policy>(wrapped, args...); }} {}
};

} // namespace qtmir

#endif // QTMIR_DISPLAYCONFIGURATIONPOLICY_H
