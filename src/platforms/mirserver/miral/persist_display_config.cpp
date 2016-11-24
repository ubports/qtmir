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
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "persist_display_config.h"

struct qtmir::miral::PersistDisplayConfig::Self {};

qtmir::miral::PersistDisplayConfig::PersistDisplayConfig() :
    self{std::make_shared<Self>()}
{
}

qtmir::miral::PersistDisplayConfig::~PersistDisplayConfig() = default;

qtmir::miral::PersistDisplayConfig::PersistDisplayConfig(PersistDisplayConfig const&) = default;

auto qtmir::miral::PersistDisplayConfig::operator=(PersistDisplayConfig const&) -> PersistDisplayConfig& = default;

void qtmir::miral::PersistDisplayConfig::operator()(mir::Server& /*server*/)
{

}
