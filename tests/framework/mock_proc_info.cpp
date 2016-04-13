/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include "mock_proc_info.h"

namespace qtmir
{

MockProcInfo::MockProcInfo()
{
    using namespace ::testing;
    ON_CALL(*this, command_line(_)).WillByDefault(Return(QByteArray()));
}

MockProcInfo::~MockProcInfo()
{
}

std::unique_ptr<qtmir::ProcInfo::CommandLine> MockProcInfo::commandLine(pid_t pid)
{
    return std::unique_ptr<CommandLine>(new CommandLine{command_line(pid)});
}

} // namespace qtmir
