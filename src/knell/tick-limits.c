#include "tick-limits.h"

/**
 * The number of ticks the driver should run.
 *
 * A value of 0 indicates to run the driver until stopped.
 */
static uint64_t tickLimit = 0;

/**
 * The number of completed ticks of the program.
 */
static uint64_t totalTicks = 0;

bool Main_IsTickLimitReached(void)
{
	return !(!tickLimit || totalTicks < tickLimit);
}

void Main_SetTickLimit(uint64_t numTicks)
{
	tickLimit = numTicks;
}

/**
 * Reports to the tick limiter that a tick occurred.
 */
void Main_TickCompleted(void)
{
	++totalTicks;
}
