#ifndef KN_TIME_H
#define KN_TIME_H

#include <knell/kn.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Functions for getting the current time and converting other units to the
 * same units being used for time.
 */
KN_API void Time_Init(void);
KN_API uint64_t Time_NowNs(void);
KN_API uint64_t Time_MsToNs(uint64_t ms);
KN_API uint64_t Time_NsToMs(uint64_t ns);
KN_API uint64_t Time_SecToNs(uint64_t sec);

#ifdef __cplusplus
}
#endif

#endif /* KN_TIME_H */
