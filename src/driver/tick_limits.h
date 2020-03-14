#ifndef KN_TICK_LIMITS_H
#define KN_TICK_LIMITS_H

#include <knell/kn.h>

bool Main_IsTickLimitReached(void);
void Main_SetTickLimit(uint64_t numTicks);
void Main_TickCompleted(void);

#endif /* KN_TICK_LIMITS_H */
