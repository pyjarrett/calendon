/*
 * Global program controls.
 */
#ifndef KN_CONTROL_H
#define KN_CONTROL_H

#include "kn.h"

/**
 * Should continue ticking?
 */
KN_API bool isRunning();

/**
 * Tells the game to gracefully shutdown.
 */
KN_API void queueGracefulShutdown();

#endif /* KN_CONTROL_H */

