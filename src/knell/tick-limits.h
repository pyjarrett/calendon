#ifndef KN_TICK_LIMITS_H
#define KN_TICK_LIMITS_H

#include <knell/kn.h>

#ifdef __cplusplus
extern "C" {
#endif

KN_API bool Main_IsTickLimitReached(void);
KN_API void Main_SetTickLimit(uint64_t numTicks);
KN_API void Main_TickCompleted(void);

#ifdef __cplusplus
}
#endif

#endif /* KN_TICK_LIMITS_H */
