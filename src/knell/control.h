#ifndef KN_CONTROL_H
#define KN_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Global program controls to initiate program shutdown.  Responsibility for
 * queueing shutdown is separated from the main driver to prevent games from
 * needing to link to anything outside of `knell`.
 */

#include <knell/kn.h>

KN_API bool Main_IsRunning(void);
KN_API void Main_QueueGracefulShutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* KN_CONTROL_H */
