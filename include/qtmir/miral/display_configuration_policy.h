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

#ifndef MIRAL_DISPLAY_CONDIFIGURATION_POLICY_H
#define MIRAL_DISPLAY_CONDIFIGURATION_POLICY_H

namespace mir { namespace graphics { class DisplayConfiguration; } }

namespace miral
{

class DisplayConfigurationPolicy
{
public:
    DisplayConfigurationPolicy();

    virtual ~DisplayConfigurationPolicy() = default;
    DisplayConfigurationPolicy(DisplayConfigurationPolicy const&) = delete;
    DisplayConfigurationPolicy& operator=(DisplayConfigurationPolicy const&) = delete;

    virtual void apply_to(mir::graphics::DisplayConfiguration& conf) = 0;
};

} // namespace miral


#endif // MIRAL_DISPLAY_CONDIFIGURATION_POLICY_H
