#include "control.h"

/**
 * Indicates whether the game should continue processing frames.
 */
bool running = true;

bool isRunning()
{
	return running;
}

void queueGracefulShutdown()
{
	running = false;
}

