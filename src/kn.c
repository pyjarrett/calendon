#include "kn.h"

#include <time.h>

uint64_t timeNowNs()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000 * 1000 * 1000 + ts.tv_nsec;
}

uint64_t timeAsMs(uint32_t ms)
{
	return ms * 1000 * 1000;
}

uint64_t timeAsSec(uint32_t sec)
{
	return timeAsMs(sec) * 1000;
}
