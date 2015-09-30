#include "mock_shared_wakelock.h"

testing::MockSharedWakelock::MockSharedWakelock(const QDBusConnection &)
{
    ON_CALL(*this, enabled()).WillByDefault(Invoke(this, &MockSharedWakelock::doEnabled));
    ON_CALL(*this, acquire(_)).WillByDefault(Invoke(this, &MockSharedWakelock::doAcquire));
    ON_CALL(*this, release(_)).WillByDefault(Invoke(this, &MockSharedWakelock::doRelease));
}

testing::MockSharedWakelock::~MockSharedWakelock()
{
}

void testing::MockSharedWakelock::doRelease(const QObject *object)
{
    if (!m_owners.remove(object)) {
        return;
    }
    if (m_owners.isEmpty()) {
        Q_EMIT enabledChanged(false);
    }
}

void testing::MockSharedWakelock::doAcquire(const QObject *object)
{
    if (m_owners.contains(object)) {
        return;
    }
    m_owners.insert(object);
    if (m_owners.size() == 1) {
        Q_EMIT enabledChanged(true);
    }
}

bool testing::MockSharedWakelock::doEnabled()
{
    return !m_owners.isEmpty();
}
