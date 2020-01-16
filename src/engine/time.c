#include "time.h"

#include <time.h>

uint64_t Time_NowNs()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000 * 1000 * 1000 + ts.tv_nsec;
}

uint64_t Time_MsToNs(uint64_t ms)
{
	return ms * 1000 * 1000;
}

uint64_t Time_SecToNs(uint64_t sec)
{
	return Time_MsToNs(sec * 1000);
}
