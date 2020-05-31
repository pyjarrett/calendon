#ifndef CALENDON_DRIVER_H
#define CALENDON_DRIVER_H

#include <calendon/cn.h>

#include <calendon/argparse.h>
#include <calendon/path.h>

#ifdef __cplusplus
extern "C" {
#endif

CN_API bool cnDriver_ParseCommandLine(int argc, char* argv[], CnDriverConfig* driverConfig);
CN_API void cnDriver_Init(CnDriverConfig* config);
CN_API void cnDriver_MainLoop(void);
CN_API void cnDriver_Shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* CALENDON_DRIVER_H */
