#include "qtmir_test.h"

namespace qtmir {

void PrintTo(const Application::InternalState& state, ::std::ostream* os) {
    switch (state) {
    case Application::InternalState::Starting:
        *os << "Starting";
        break;
    case Application::InternalState::Running:
        *os << "Running";
        break;
    case Application::InternalState::RunningWithoutWakelock:
        *os << "RunningWithoutWakelock";
        break;
    case Application::InternalState::SuspendingWaitSession:
        *os << "SuspendingWaitSession";
        break;
    case Application::InternalState::SuspendingWaitProcess:
        *os << "SuspendingWaitProcess";
        break;
    case Application::InternalState::Suspended:
        *os << "Suspended";
        break;
    case Application::InternalState::KilledOutOfMemory:
        *os << "KilledOutOfMemory";
        break;
    case Application::InternalState::Stopped:
        *os << "Stopped";
        break;
    default:
        *os << "???";
        break;
    }
}

void PrintTo(const Session::State& state, ::std::ostream* os)
{
    switch (state) {
    case SessionInterface::Starting:
        *os << "Starting";
        break;
    case SessionInterface::Running:
        *os << "Running";
        break;
    case SessionInterface::Suspending:
        *os << "Suspending";
        break;
    case SessionInterface::Suspended:
        *os << "Suspended";
        break;
    case SessionInterface::Stopped:
        *os << "Stopped";
        break;
    default:
        *os << "???";
        break;
    }
}

} // namespace qtmir
