#ifndef KN_GAME_H
#define KN_GAME_H

/**
 * Interface to be implemented by games and demos.
 *
 * Games start by having their `Game_Init` function called.  Every tick, both
 * `Game_Tick` and `Game_Draw` are called in that order.  Immediately before the
 * game is unloaded `Game_Shutdown` is called.
 *
 * `Game_Init` may indicate failure in the payload initializing by returning
 * false.  `KN_FATAL_ERROR` may also be used to indicate failures in Knell
 * related components.
 */

#include <knell/kn.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*Game_InitPROC)(void);
typedef void (*Game_DrawPROC)(void);
typedef void (*Game_TickPROC)(uint64_t);
typedef void (*Game_ShutdownPROC)(void);

KN_API Game_InitPROC Game_InitFn;
KN_API Game_DrawPROC Game_DrawFn;
KN_API Game_TickPROC Game_TickFn;
KN_API Game_ShutdownPROC Game_ShutdownFn;

/**
 * Loads a shared library's functions appropriately for the current platform.
 */
KN_API void Game_Load(const char* sharedLibraryName);

#ifdef __cplusplus
}
#endif

#endif /* KN_GAME_H */
