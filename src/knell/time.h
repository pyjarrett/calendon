#ifndef KN_TIME_H
#define KN_TIME_H

#include <stdint.h>

/*
 * Functions for getting the current time and converting other units to the
 * same units being used for time.
 */
void Time_Init(void);
uint64_t Time_NowNs(void);
uint64_t Time_MsToNs(uint64_t ms);
uint64_t Time_SecToNs(uint64_t sec);

#endif /* KN_TIME_H */
