#include "mock_proc_info.h"

namespace qtmir
{

MockProcInfo::MockProcInfo()
{
}

MockProcInfo::~MockProcInfo()
{
}

std::unique_ptr<qtmir::ProcInfo::CommandLine> MockProcInfo::commandLine(quint64 pid)
{
    return std::unique_ptr<CommandLine>(new CommandLine{command_line(pid)});
}

} // namespace qtmir
