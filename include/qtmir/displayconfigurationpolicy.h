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

// Qt
#include <QScopedPointer>

// mir
#include <mir/graphics/display_configuration_policy.h>
#include <mir/graphics/display_configuration.h>
#include <mir/server.h>

#include <memory>

////////////////////// SHOULD BE IN MIRAL //////////////////
namespace miral {

class BasicSetDisplayConfigurationPolicy
{
public:
    explicit BasicSetDisplayConfigurationPolicy(std::function<std::shared_ptr<mir::graphics::DisplayConfigurationPolicy>(
                                                    std::shared_ptr<mir::graphics::DisplayConfigurationPolicy> const&)> const& builder);
    ~BasicSetDisplayConfigurationPolicy() = default;

    void operator()(mir::Server& server);
    auto the_display_configuration_policy() const -> std::shared_ptr<mir::graphics::DisplayConfigurationPolicy>;

private:
    struct Private;
    std::shared_ptr<Private> d;
};

template<typename Policy>
class SetDisplayConfigurationPolicy : public BasicSetDisplayConfigurationPolicy
{
public:
    template<typename ...Args>
    explicit SetDisplayConfigurationPolicy(Args const& ...args)
        : BasicSetDisplayConfigurationPolicy{
            [&args...](std::shared_ptr<mir::graphics::DisplayConfigurationPolicy> const& wrapped) {
                return std::make_shared<Policy>(wrapped, args...);
        }}
    {}
};

} // namespace miral
////////////////////////////////////////////////////////////

class QMirServer;

namespace qtmir {

//Conver to miral::DisplayConfigurationPolicy.
class DisplayConfigurationPolicy : public mir::graphics::DisplayConfigurationPolicy
{
public:
    DisplayConfigurationPolicy(std::shared_ptr<mir::graphics::DisplayConfigurationPolicy> const& wrapped);

    void apply_to(mir::graphics::DisplayConfiguration& conf);

private:
    struct Private;
    std::shared_ptr<Private> d;
};

class BasicSetDisplayConfigurationPolicy
{
public:
    explicit BasicSetDisplayConfigurationPolicy(miral::BasicSetDisplayConfigurationPolicy const& builder);
    ~BasicSetDisplayConfigurationPolicy() = default;

    void operator()(QMirServer& server);

private:
    struct Private;
    std::shared_ptr<Private> d;
};

template<typename Policy>
class SetDisplayConfigurationPolicy : public BasicSetDisplayConfigurationPolicy
{
public:
    template<typename ...Args>
    explicit SetDisplayConfigurationPolicy(Args const& ...args) :
            BasicSetDisplayConfigurationPolicy{miral::SetDisplayConfigurationPolicy<Policy>(args...)} {}
};

auto wrapDisplayConfigurationPolicy(const std::shared_ptr<mir::graphics::DisplayConfigurationPolicy> &wrapped)
-> std::shared_ptr<mir::graphics::DisplayConfigurationPolicy>;

} // namespace qtmir

#endif // QTMIR_DISPLAYCONFIGURATIONPOLICY_H
