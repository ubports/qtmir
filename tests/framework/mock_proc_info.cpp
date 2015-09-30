#include "mock_proc_info.h"

testing::MockProcInfo::MockProcInfo()
{
}

testing::MockProcInfo::~MockProcInfo()
{
}

std::unique_ptr<qtmir::ProcInfo::CommandLine> testing::MockProcInfo::commandLine(quint64 pid)
{
    return std::unique_ptr<CommandLine>(new CommandLine{command_line(pid)});
}
