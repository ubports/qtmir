#include "stub_input_channel.h"

mir::test::doubles::StubInputChannel::StubInputChannel(int fd)
    : input_fd(fd)
{
}

mir::test::doubles::StubInputChannel::StubInputChannel()
    : StubInputChannel(0)
{
}

mir::test::doubles::StubInputChannel::~StubInputChannel()
{
}

int mir::test::doubles::StubInputChannel::client_fd() const
{
    return input_fd;
}

int mir::test::doubles::StubInputChannel::server_fd() const
{
    return input_fd;
}
