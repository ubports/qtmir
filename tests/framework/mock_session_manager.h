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

#ifndef MOCK_SESSION_MANAGER_H
#define MOCK_SESSION_MANAGER_H

#include <Unity/Application/sessionmanager.h>

#include <gmock/gmock.h>

namespace qtmir {

struct MockSessionManager : public qtmir::SessionManager
{
    MOCK_CONST_METHOD1(findSession, qtmir::SessionInterface*(const mir::scene::Session*));
};

} // namespace qtmir

#endif // MOCK_SESSION_MANAGER_H
