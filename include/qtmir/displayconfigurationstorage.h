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

#ifndef DISPLAYCONFIGURATIONSTORAGE_H
#define DISPLAYCONFIGURATIONSTORAGE_H

#include "qtmir/miral/display_configuration_storage.h"

#include <functional>
#include <memory>

class QMirServer;

namespace qtmir
{

using DisplayConfigurationStorageBuilder = std::function<std::shared_ptr<miral::DisplayConfigurationStorage>()>;

class BasicSetDisplayConfigurationStorage
{
public:
    explicit BasicSetDisplayConfigurationStorage(DisplayConfigurationStorageBuilder const& builder);
    ~BasicSetDisplayConfigurationStorage() = default;

    void operator()(QMirServer& server);
    DisplayConfigurationStorageBuilder builder() const;

private:
    struct Self;
    std::shared_ptr<Self> self;
};

/*
    Set the display configuration policy to allow server customization
 */
template<typename Policy>
class SetDisplayConfigurationStorage : public BasicSetDisplayConfigurationStorage
{
public:
    template<typename ...Args>
    explicit SetDisplayConfigurationStorage(Args const& ...args) :
        BasicSetDisplayConfigurationStorage{
            [&args...]() { return std::make_shared<Policy>(args...); }} {}
};

}

#endif // DISPLAYCONFIGURATIONSTORAGE_H
