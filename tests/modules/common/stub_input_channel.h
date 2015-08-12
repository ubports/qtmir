/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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

#ifndef MIR_TEST_DOUBLES_STUB_INPUT_CHANNEL_H_
#define MIR_TEST_DOUBLES_STUB_INPUT_CHANNEL_H_

#include "mir/input/input_channel.h"

namespace mir
{
namespace test
{
namespace doubles
{

struct StubInputChannel : public input::InputChannel
{
    StubInputChannel(int fd)
      : input_fd(fd)
    {
    }

    StubInputChannel()
     : StubInputChannel(0)
    {
    }

    int client_fd() const override
    {
        return input_fd;
    }
    int server_fd() const override
    {
        return input_fd;
    }
    int input_fd;
};

}
}
} // namespace mir

#endif // MIR_TEST_DOUBLES_STUB_INPUT_CHANNEL_H_

