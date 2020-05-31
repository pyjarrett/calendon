#ifndef CN_TICK_LIMITS_H
#define CN_TICK_LIMITS_H

#include <calendon/cn.h>

#ifdef __cplusplus
extern "C" {
#endif

CN_API bool cnMain_IsTickLimitReached(void);
CN_API void cnMain_SetTickLimit(uint64_t numTicks);
CN_API void cnMain_TickCompleted(void);

#ifdef __cplusplus
}
#endif

#endif /* CN_TICK_LIMITS_H */
