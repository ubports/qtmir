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

#include "qtmir/displayconfigurationpolicy.h"
#include "qmirserver.h"

namespace qtmir
{

struct BasicSetDisplayConfigurationStorage::Self
{
    Self(DisplayConfigurationStorageBuilder const& builder) :
        builder{builder} {}

    DisplayConfigurationStorageBuilder builder;
};

BasicSetDisplayConfigurationStorage::BasicSetDisplayConfigurationStorage(const DisplayConfigurationStorageBuilder &builder) :
    self(std::make_shared<Self>(builder))
{
}

void BasicSetDisplayConfigurationStorage::operator()(QMirServer &server)
{
    server.overrideDisplayConfigurationStorage(*this);
}

DisplayConfigurationStorageBuilder BasicSetDisplayConfigurationStorage::builder() const
{
    return self->builder;
}

}
