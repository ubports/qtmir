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

#ifndef MOCK_SESSIONMAP_H
#define MOCK_SESSIONMAP_H

#include <gmock/gmock.h>

#include <Unity/Application/sessionmap_interface.h>

struct MockSessionMap : public qtmir::SessionMapInterface
{
    MockSessionMap() {}
    virtual ~MockSessionMap() {}

    MOCK_CONST_METHOD1(findSession, SessionInterface*(const mir::scene::Session*));
};

#endif // MOCK_SESSIONMAP_H
