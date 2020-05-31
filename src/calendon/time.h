#ifndef CN_TIME_H
#define CN_TIME_H

#include <calendon/cn.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Functions for getting the current time and converting other units to the
 * same units being used for time.
 */
CN_API void cnTime_Init(void);
CN_API uint64_t cnTime_NowNs(void);
CN_API uint64_t cnTime_MsToNs(uint64_t ms);
CN_API uint64_t cnTime_NsToMs(uint64_t ns);
CN_API uint64_t cnTime_SecToNs(uint64_t sec);

#ifdef __cplusplus
}
#endif

#endif /* CN_TIME_H */
