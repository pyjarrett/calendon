/*
 * Global program controls.
 */
#ifndef KN_CONTROL_H
#define KN_CONTROL_H

#include <stdbool.h>

/**
 * Should continue ticking?
 */
bool isRunning();

/**
 * Tells the game to gracefully shutdown.
 */
void queueGracefulShutdown();

#endif /* KN_CONTROL_H */

