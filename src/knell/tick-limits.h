#ifndef KN_TICK_LIMITS_H
#define KN_TICK_LIMITS_H

#include <knell/kn.h>

KN_API bool Main_IsTickLimitReached(void);
KN_API void Main_SetTickLimit(uint64_t numTicks);
KN_API void Main_TickCompleted(void);

#endif /* KN_TICK_LIMITS_H */
