#include "stub_input_channel.h"

namespace mir
{
namespace test
{
namespace doubles
{

StubInputChannel::StubInputChannel(int fd)
    : input_fd(fd)
{
}

StubInputChannel::StubInputChannel()
    : StubInputChannel(0)
{
}

StubInputChannel::~StubInputChannel()
{
}

int StubInputChannel::client_fd() const
{
    return input_fd;
}

int StubInputChannel::server_fd() const
{
    return input_fd;
}

} // namespace doubles
} // namespace test
} // namespace mir
