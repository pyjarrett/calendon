/*
 * Global program controls.
 */
#ifndef KN_CONTROL_H
#define KN_CONTROL_H

#include "kn.h"

/**
 * Should continue ticking?
 */
KN_API bool Main_IsRunning(void);

/**
 * Tells the game to gracefully shutdown.
 */
KN_API void Main_QueueGracefulShutdown(void);

#endif /* KN_CONTROL_H */

