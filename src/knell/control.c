#include "control.h"

/**
 * Indicates whether the game should continue processing frames.
 */
bool running = true;

KN_API bool isRunning()
{
	return running;
}

KN_API void queueGracefulShutdown()
{
	running = false;
}
