#include "time.h"

#include <calendon/float.h>
#include <calendon/system.h>

#ifdef _WIN32
// Windows lacks the convenient POSIX API for this, so resort to using query
// performance counters (QPC).
// https://docs.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps
#include <calendon/compat-windows.h>

LARGE_INTEGER qpcFrequency;

bool cnTime_Init(void)
{
	// From the docs:
	// The qpcFrequency of the performance counter is fixed at system boot and is
	// consistent across all processors so you only need to query the qpcFrequency
	// from QueryPerformanceFrequency as the application initializes, and then
	// cache the result.
	QueryPerformanceFrequency(&qpcFrequency);
	return true;
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

bool cnTime_Init(void)
{
	return true;
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

uint64_t cnUInt64_SubtractMonotonic(uint64_t left, uint64_t right)
{
	if (left < right) return 0;
	return left - right;
}

CnTime cnTime_MakeNow(void)
{
	return (CnTime) { .native = cnTime_NowNs() };
}

CnTime cnTime_MakeMilli(uint64_t millis)
{
	return (CnTime) { .native = cnTime_MsToNs(millis) };
}

CnTime cnTime_MakeZero(void)
{
	return (CnTime) { .native = 0ULL };
}

uint64_t cnTime_Milli(CnTime t)
{
	return cnTime_NsToMs(t.native);
}

bool cnTime_IsZero(CnTime t)
{
	return t.native == 0ULL;
}

CnTime cnTime_Add(CnTime left, CnTime right)
{
	return (CnTime) { left.native + right.native };
}

/**
 * Subtract, but do not underflow.  If underflow would occur, return a time
 * of zero.
 */
CnTime cnTime_SubtractMonotonic(CnTime left, CnTime right)
{
	return (CnTime) { cnUInt64_SubtractMonotonic(left.native, right.native) };
}

/**
 * Linearly interpolates from 0 to 1, when going from zero to a given duration.
 */
float cnTime_Lerp(CnTime currentDuration, CnTime totalDuration)
{
	CN_ASSERT(!cnTime_IsZero(totalDuration), "Cannot LERP against a zero total time.");
	return cnFloat_Clamp((float)currentDuration.native / totalDuration.native,
		0.0f, 1.0f);
}

bool cnTime_LessThan(CnTime left, CnTime right)
{
	return left.native < right.native;
}

CnTime cnTime_Max(CnTime left, CnTime right)
{
	return cnTime_LessThan(left, right) ? right : left;
}

CnTime cnTime_Min(CnTime left, CnTime right)
{
	return cnTime_LessThan(left, right) ? left : right;
}

CnBehavior cnTime_Plugin(void)
{
	return (CnBehavior) {
		.init = cnTime_Init,
		.shutdown = NULL,
		.tick = NULL,
		.draw = NULL,
		.sharedLibrary = NULL
	};
}

CnSystem cnTime_System(void)
{
	return (CnSystem) {
		.name = "Time",
		.options = cnSystem_NoOptions,
		.setDefaultConfig = cnSystem_NoDefaultConfig,
		.config = cnSystem_NoConfig,
		.plugin = cnTime_Plugin
	};
}
