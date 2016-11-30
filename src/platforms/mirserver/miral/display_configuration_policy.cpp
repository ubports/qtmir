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

#include <mir/graphics/display_configuration_policy.h>
#include <mir/server.h>

namespace mg = mir::graphics;

namespace
{

struct MirWrappedMiralDisplayConfigurationPolicy : mg::DisplayConfigurationPolicy
{
    MirWrappedMiralDisplayConfigurationPolicy(std::shared_ptr<qtmir::miral::DisplayConfigurationPolicy> const& self) :
        self{self}
    {}

    void apply_to(mg::DisplayConfiguration& conf) override
    {
        self->apply_to(conf);
    }

    std::shared_ptr<qtmir::miral::DisplayConfigurationPolicy> self;
};

struct MiralWrappedMirDisplayConfigurationPolicy : qtmir::miral::DisplayConfigurationPolicy
{
    MiralWrappedMirDisplayConfigurationPolicy(std::shared_ptr<mg::DisplayConfigurationPolicy> const& self) :
        qtmir::miral::DisplayConfigurationPolicy(nullptr),
        self{self}
    {}

    void apply_to(mg::DisplayConfiguration& conf) override
    {
        self->apply_to(conf);
    }

    std::shared_ptr<mg::DisplayConfigurationPolicy> self;
};

}

struct qtmir::miral::DisplayConfigurationPolicy::Self
{
    Self(const std::shared_ptr<qtmir::miral::DisplayConfigurationPolicy> &wrapped) :
        wrapped{wrapped}
    {}

    std::shared_ptr<qtmir::miral::DisplayConfigurationPolicy> wrapped;
};

qtmir::miral::DisplayConfigurationPolicy::DisplayConfigurationPolicy(std::shared_ptr<qtmir::miral::DisplayConfigurationPolicy> const& wrapped)
    : self{std::make_shared<Self>(wrapped)}
{
}

void qtmir::miral::DisplayConfigurationPolicy::apply_to(mir::graphics::DisplayConfiguration& conf)
{
    if (self->wrapped) {
        self->wrapped->apply_to(conf);
    }
}

std::shared_ptr<qtmir::miral::DisplayConfigurationPolicy> qtmir::miral::DisplayConfigurationPolicy::wrapped_policy() const
{
    return self->wrapped;
}
