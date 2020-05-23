#ifndef KNELL_DRIVER_H
#define KNELL_DRIVER_H

#include <knell/kn.h>

KN_API void knDriver_Init(int argc, char** argv);
KN_API void knDriver_MainLoop(void);
KN_API void knDriver_Shutdown(void);

#endif /* KNELL_DRIVER_H */
