#ifndef KNELL_DRIVER_H
#define KNELL_DRIVER_H

#include <knell/kn.h>

#ifdef __cplusplus
extern "C" {
#endif

KN_API void knDriver_Init(int argc, char** argv);
KN_API void knDriver_MainLoop(void);
KN_API void knDriver_Shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* KNELL_DRIVER_H */
