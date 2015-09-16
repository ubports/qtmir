#include <QCoreApplication>
#include <QVariant>

namespace qtmir {

namespace {

void resetStartTime(std::chrono::nanoseconds timestamp)
{
    if (qApp) {
        qApp->setProperty("appStartTime", (qint64)timestamp.count());
    }
}
std::chrono::nanoseconds getStartTime(std::chrono::nanoseconds timestamp, bool allowReset = true)
{
    if (!qApp) return std::chrono::nanoseconds(0);
    QVariant vData = qApp->property("appStartTime");
    if (!vData.isValid()) {
        if (allowReset) {
            resetStartTime(timestamp);
            return timestamp;
        }
        return std::chrono::nanoseconds(0);
    }
    return std::chrono::nanoseconds(vData.value<qint64>());
}

}

template<typename T>
T compressTimestamp(std::chrono::nanoseconds timestamp)
{
    std::chrono::nanoseconds startTime = getStartTime(timestamp);
    std::chrono::nanoseconds result = timestamp - startTime;

    if (std::numeric_limits<std::chrono::nanoseconds::rep>::max() > std::numeric_limits<T>::max() &&
        result > std::chrono::nanoseconds(std::numeric_limits<T>::max())) {
        // we've overflowed the boundaries of the millisecond type.
        resetStartTime(timestamp);
        return 0;
    }

    return result.count();
}

template<typename T>
std::chrono::nanoseconds uncompressTimestamp(T timestamp)
{
    auto tsNS = std::chrono::nanoseconds(timestamp);
    return getStartTime(tsNS, false) + std::chrono::nanoseconds(tsNS);
}

}