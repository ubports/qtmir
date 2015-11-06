#include <QCoreApplication>
#include <QVariant>

extern "C" {
	void resetStartTime(std::chrono::nanoseconds timestamp);
	std::chrono::nanoseconds getStartTime(std::chrono::nanoseconds timestamp, bool allowReset = true);
}

namespace qtmir {

typedef std::chrono::duration<ulong, std::milli> Timestamp;

template<typename T>
T compressTimestamp(std::chrono::nanoseconds timestamp)
{
    std::chrono::nanoseconds startTime = getStartTime(timestamp);

    if (std::chrono::nanoseconds::max() > T::max() &&
        timestamp - startTime > std::chrono::nanoseconds(T::max())) {
        // we've overflowed the boundaries of the millisecond type.
        resetStartTime(timestamp);
        return T(0);
    }

    return std::chrono::duration_cast<T>(timestamp - startTime);
}

template<typename T>
std::chrono::nanoseconds uncompressTimestamp(T timestamp)
{
    auto tsNS = std::chrono::nanoseconds(timestamp);
    return getStartTime(tsNS, false) + std::chrono::nanoseconds(tsNS);
}

}