#ifndef CN_MAIN_H
#define CN_MAIN_H

/**
 * @file main.h
 *
 * The main program controls for Calendon.  These are the high-level controls to
 * run a Calendon-based program.
 */

#include <calendon/cn.h>

#ifdef __cplusplus
extern "C" {
#endif

CN_API void cnMain_StartUp(int argc, char** argv);
CN_API void cnMain_Loop(void);
CN_API void cnMain_Shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* CN_MAIN_H */
