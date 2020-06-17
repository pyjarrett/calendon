#include "time.h"

#ifdef _WIN32
// Windows lacks the convenient POSIX API for this, so resort to using query
// performance counters (QPC).
// https://docs.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps
#include <calendon/compat-windows.h>

LARGE_INTEGER qpcFrequency;

void cnTime_Init(void)
{
	// From the docs:
	// The qpcFrequency of the performance counter is fixed at system boot and is
	// consistent across all processors so you only need to query the qpcFrequency
	// from QueryPerformanceFrequency as the application initializes, and then
	// cache the result.
	QueryPerformanceFrequency(&qpcFrequency);
}

uint64_t cnTime_NowNs(void)
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

void cnTime_Init(void)
{
}

uint64_t cnTime_NowNs(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000 * 1000 * 1000 + (uint64_t)ts.tv_nsec;
}

#endif /* _WIN32 */

uint64_t cnTime_MsToNs(uint64_t ms)
{
	return ms * 1000 * 1000;
}

uint64_t cnTime_NsToMs(uint64_t ns)
{
	return ns / (1000 * 1000);
}

uint64_t cnTime_SecToNs(uint64_t sec)
{
	return cnTime_MsToNs(sec * 1000);
}

uint64_t cnTime_MonotonicSubtract(uint64_t left, uint64_t right)
{
	if (left < right) return 0;
	return left - right;
}