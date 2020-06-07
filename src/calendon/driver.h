#ifndef CN_DRIVER_H
#define CN_DRIVER_H

#include <calendon/cn.h>

#include <calendon/argparse.h>
#include <calendon/path.h>

#ifdef __cplusplus
extern "C" {
#endif

CN_API bool cnDriverConfig_ParseCommandLine(CnDriverConfig* driverConfig, int argc, char* argv[]);
CN_API void cnDriverConfig_Freestanding(CnDriverConfig* config, CnPlugin_InitFn init, CnPlugin_TickFn tick, CnPlugin_DrawFn draw, CnPlugin_ShutdownFn shutdown);

CN_API void cnDriver_Init(CnDriverConfig* config);
CN_API void cnDriver_MainLoop(void);
CN_API void cnDriver_Shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* CALENDON_DRIVER_H */
