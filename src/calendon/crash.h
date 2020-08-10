/*
 * Crash handlers and core dumps.
 */
#ifndef CN_CRASH_H
#define CN_CRASH_H

#include <calendon/cn.h>
#include <calendon/system.h>

#ifdef __cplusplus
extern "C" {
#endif

CN_API bool cnCrash_Init(void);

CnSystem cnCrash_System(void);

#ifdef __cplusplus
}
#endif

#endif /* CN_CRASH_H */
