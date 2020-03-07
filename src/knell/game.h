#ifndef KN_GAME_H
#define KN_GAME_H

#include <knell/kn.h>

typedef void (*Game_InitPROC)(void);
typedef void (*Game_DrawPROC)(void);
typedef void (*Game_TickPROC)(uint64_t);
typedef void (*Game_ShutdownPROC)(void);

KN_API Game_InitPROC Game_InitFn;
KN_API Game_DrawPROC Game_DrawFn;
KN_API Game_TickPROC Game_TickFn;
KN_API Game_ShutdownPROC Game_ShutdownFn;

KN_API void Game_Load(const char* sharedLibraryName);

#endif /* KN_GAME_H */
