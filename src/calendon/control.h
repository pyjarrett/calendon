#ifndef CN_CONTROL_H
#define CN_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Global program controls to initiate program shutdown.  Responsibility for
 * queueing shutdown is separated from the main driver to prevent games from
 * needing to link to anything outside of `calendon`.
 */

#include <calendon/cn.h>

CN_API bool cnMain_IsRunning(void);
CN_API void cnMain_QueueGracefulShutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* CN_CONTROL_H */
