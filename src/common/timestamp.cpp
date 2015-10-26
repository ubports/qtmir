
#include <chrono>
#include "timestamp_impl.h"

std::chrono::nanoseconds appStartTime(0);

void resetStartTime(std::chrono::nanoseconds timestamp)
{
	appStartTime = timestamp;
}

std::chrono::nanoseconds getStartTime(std::chrono::nanoseconds timestamp, bool allowReset)
{
	if (allowReset && appStartTime.count() == 0) {
		resetStartTime(timestamp);
	}
	return appStartTime;
}
