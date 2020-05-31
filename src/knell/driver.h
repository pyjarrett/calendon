#ifndef KNELL_DRIVER_H
#define KNELL_DRIVER_H

#include <knell/kn.h>

#include <knell/argparse.h>
#include <knell/path.h>

#ifdef __cplusplus
extern "C" {
#endif

KN_API bool knDriver_ParseCommandLine(int argc, char* argv[], knDriverConfig* driverConfig);
KN_API void knDriver_Init(knDriverConfig* config);
KN_API void knDriver_MainLoop(void);
KN_API void knDriver_Shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* KNELL_DRIVER_H */
