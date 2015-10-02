#include "mock_shared_wakelock.h"

namespace qtmir
{

MockSharedWakelock::MockSharedWakelock(const QDBusConnection &)
{
    using namespace ::testing;

    ON_CALL(*this, enabled()).WillByDefault(Invoke(this, &MockSharedWakelock::doEnabled));
    ON_CALL(*this, acquire(_)).WillByDefault(Invoke(this, &MockSharedWakelock::doAcquire));
    ON_CALL(*this, release(_)).WillByDefault(Invoke(this, &MockSharedWakelock::doRelease));
}

MockSharedWakelock::~MockSharedWakelock()
{
}

void MockSharedWakelock::doRelease(const QObject *object)
{
    if (!m_owners.remove(object)) {
        return;
    }
    if (m_owners.isEmpty()) {
        Q_EMIT enabledChanged(false);
    }
}

void MockSharedWakelock::doAcquire(const QObject *object)
{
    if (m_owners.contains(object)) {
        return;
    }
    m_owners.insert(object);
    if (m_owners.size() == 1) {
        Q_EMIT enabledChanged(true);
    }
}

bool MockSharedWakelock::doEnabled()
{
    return !m_owners.isEmpty();
}

} // namespace qtmir
