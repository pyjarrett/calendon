#ifndef KN_CONTROL_H
#define KN_CONTROL_H

/*
 * Global program controls to initiate program shutdown.  Responsibility for
 * queueing shutdown is separated from the main driver to prevent games from
 * needing to link to anything outside of `knell-lib`.
 */

#include <knell/kn.h>

KN_API bool Main_IsRunning(void);
KN_API void Main_QueueGracefulShutdown(void);

#endif /* KN_CONTROL_H */
