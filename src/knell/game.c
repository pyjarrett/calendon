#include "game.h"

#include <knell/kn.h>

#include <knell/shared-library.h>

static knSharedLibrary GameModule;

KN_API void Game_Load(const char* sharedLibraryName)
{
	if (Game_ShutdownFn) {
		Game_ShutdownFn();
	}

	SharedLibrary_Release(GameModule);
	GameModule = SharedLibrary_Load(sharedLibraryName);
	if (!GameModule) {
		KN_FATAL_ERROR("Unable to load game module: %s", sharedLibraryName);
	}
	Game_InitFn = (Game_InitPROC)SharedLibrary_LookupFn(GameModule, "Game_Init");
	if (!Game_InitFn) {
		KN_FATAL_ERROR("Game_Init function missing in %s", sharedLibraryName);
	}
	Game_DrawFn = (Game_DrawPROC)SharedLibrary_LookupFn(GameModule, "Game_Draw");
	if (!Game_DrawFn) {
		KN_FATAL_ERROR("Game_DrawFn function missing in %s", sharedLibraryName);
	}
	Game_TickFn = (Game_TickPROC)SharedLibrary_LookupFn(GameModule, "Game_Tick");
	if (!Game_TickFn) {
		KN_FATAL_ERROR("Game_TickFn function missing in %s", sharedLibraryName);
	}
	Game_ShutdownFn = (Game_ShutdownPROC)SharedLibrary_LookupFn(GameModule, "Game_Shutdown");
	if (!Game_ShutdownFn) {
		KN_FATAL_ERROR("Game_ShutdownFn function missing in %s", sharedLibraryName);
	}

	if (!Game_InitFn()) {
		KN_FATAL_ERROR("%s failed to initialize", sharedLibraryName);
	}
}
