/*
 * Copyright (C) 2015-2016 Canonical, Ltd.
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

#ifndef MIRDISPLAYCONFIGURATIONPOLICY_H
#define MIRDISPLAYCONFIGURATIONPOLICY_H

#include <mir/graphics/display_configuration_policy.h>

#include <memory>

class MirDisplayConfigurationPolicy : public mir::graphics::DisplayConfigurationPolicy
{
public:
    MirDisplayConfigurationPolicy(const std::shared_ptr<mir::graphics::DisplayConfigurationPolicy> &wrapped);

    void apply_to(mir::graphics::DisplayConfiguration &conf) override;

private:
    const std::shared_ptr<mir::graphics::DisplayConfigurationPolicy> m_wrapped;
};

#endif // MIRDISPLAYCONFIGURATIONPOLICY_H
