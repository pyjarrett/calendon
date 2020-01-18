#include "control.h"

/**
 * Indicates whether the game should continue processing frames.
 */
static bool running = true;

KN_API bool Main_IsRunning(void)
{
	return running;
}

KN_API void Main_QueueGracefulShutdown(void)
{
	running = false;
}
