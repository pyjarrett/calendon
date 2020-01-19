#include "time.h"

#ifdef _WIN32
// Windows lacks the convenient POSIX API for this, so resort to using query
// performance counters (QPC).
// https://docs.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps
#include "compat-windows.h"

LARGE_INTEGER qpcFrequency;

KN_API void Time_Init(void)
{
	// From the docs:
	// The qpcFrequency of the performance counter is fixed at system boot and is
	// consistent across all processors so you only need to query the qpcFrequency
	// from QueryPerformanceFrequency as the application initializes, and then
	// cache the result.
	QueryPerformanceFrequency(&qpcFrequency);
}

KN_API uint64_t Time_NowNs(void)
{
	// QPC gives the number of ticks, need to convert to nanoseconds.
	LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);

	ticks.QuadPart *= (1000 * 1000 * 1000);
	ticks.QuadPart /= qpcFrequency.QuadPart;

	return ticks.QuadPart;
}

#else

#include <time.h>

KN_API void Time_Init(void)
{
}

KN_API uint64_t Time_NowNs(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000 * 1000 * 1000 + (uint64_t)ts.tv_nsec;
}

#endif /* _WIN32 */

KN_API uint64_t Time_MsToNs(uint64_t ms)
{
	return ms * 1000 * 1000;
}

KN_API uint64_t Time_SecToNs(uint64_t sec)
{
	return Time_MsToNs(sec * 1000);
}
