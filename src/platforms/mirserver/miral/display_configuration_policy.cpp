/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Nick Dedekind <nick.dedekind@canonical.com>
 */

#include "qtmir/miral/display_configuration_policy.h"

struct miral::experimental::DisplayConfigurationPolicy::Self
{
    Self(const std::shared_ptr<miral::experimental::DisplayConfigurationPolicy> &wrapped) :
        wrapped{wrapped}
    {}

    std::shared_ptr<miral::experimental::DisplayConfigurationPolicy> wrapped;
};

miral::experimental::DisplayConfigurationPolicy::DisplayConfigurationPolicy(std::shared_ptr<miral::experimental::DisplayConfigurationPolicy> const& wrapped)
    : self{std::make_shared<Self>(wrapped)}
{
}

std::shared_ptr<miral::experimental::DisplayConfigurationPolicy> miral::experimental::DisplayConfigurationPolicy::wrapped_policy() const
{
    return self->wrapped;
}
