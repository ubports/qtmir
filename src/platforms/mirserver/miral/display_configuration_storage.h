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

#ifndef MIRAL_DISPLAY_CONFIGURATION_STORAGE_H
#define MIRAL_DISPLAY_CONFIGURATION_STORAGE_H

#include "mir/geometry/rectangle.h"
#include "mir/optional_value.h"
#include "mir_toolkit/common.h"

// Prototyping namespace for later incorporation in MirAL
namespace miral
{
class Edid;

struct DisplayOutputOptions
{
    mir::optional_value<bool> used;
    mir::optional_value<uint> clone_of;
    mir::optional_value<mir::geometry::Size> size;
    mir::optional_value<MirOrientation> orientation;
    mir::optional_value<MirFormFactor> form_factor;
    mir::optional_value<float> scale;
};

class DisplayConfigurationStorage
{
public:
    virtual ~DisplayConfigurationStorage() = default;

    virtual void save(const Edid&, const DisplayOutputOptions&) = 0;
    virtual bool load(const Edid&, DisplayOutputOptions&) const = 0;
};

}

#endif // MIRAL_DISPLAY_CONFIGURATION_STORAGE_H
