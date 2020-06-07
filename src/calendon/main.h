#ifndef CN_DRIVER_H
#define CN_DRIVER_H

#include <calendon/cn.h>

#include <calendon/main-config.h>

#ifdef __cplusplus
extern "C" {
#endif

CN_API void cnMain_Init(CnMainConfig* config);
CN_API void cnMain_Loop(void);
CN_API void cnMain_Shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* CALENDON_DRIVER_H */
