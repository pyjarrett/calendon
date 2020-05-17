#include "game.h"

#include <knell/kn.h>

#ifdef _WIN32

#include <knell/compat-windows.h>
#include <knell/process.h>
static HMODULE GameModule;

KN_API void Game_Load(const char* sharedLibraryName)
{
	// Shutdown any previous game.
	if (Game_ShutdownFn) {
		Game_ShutdownFn();
	}

	if (GameModule) {
		FreeLibrary(GameModule);
	}

	GameModule = LoadLibrary(sharedLibraryName);
	if (!GameModule) {
		KN_FATAL_ERROR("Unable to load game module: %s", sharedLibraryName);
	}
	Game_InitFn = (Game_InitPROC)GetProcAddress(GameModule, "Game_Init");
	if (!Game_InitFn) {
		KN_FATAL_ERROR("Game_Init function missing in %s", sharedLibraryName);
	}
	Game_DrawFn = (Game_DrawPROC)GetProcAddress(GameModule, "Game_Draw");
	if (!Game_DrawFn) {
		KN_FATAL_ERROR("Game_DrawFn function missing in %s", sharedLibraryName);
	}
	Game_TickFn = (Game_TickPROC)GetProcAddress(GameModule, "Game_Tick");
	if (!Game_TickFn) {
		KN_FATAL_ERROR("Game_TickFn function missing in %s", sharedLibraryName);
	}
	Game_ShutdownFn = (Game_ShutdownPROC)GetProcAddress(GameModule, "Game_Shutdown");
	if (!Game_ShutdownFn) {
		KN_FATAL_ERROR("Game_ShutdownFn function missing in %s", sharedLibraryName);
	}

	Game_InitFn();
}

#endif /* WIN32 */

#ifdef __linux__

#include <dlfcn.h>

static void* GameModule;

void Game_Load(const char* sharedLibraryName)
{
	// Shutdown any previous game.
	if (Game_ShutdownFn) {
		Game_ShutdownFn();
	}

	if (GameModule) {
		dlclose(GameModule);
	}

	GameModule = dlopen(sharedLibraryName,  RTLD_NOW);
	if (!GameModule) {
		KN_FATAL_ERROR("Unable to load game module: %s", sharedLibraryName);
	}
	Game_InitFn = (Game_InitPROC)dlsym(GameModule, "Game_Init");
	Game_DrawFn = (Game_DrawPROC)dlsym(GameModule, "Game_Draw");
	Game_TickFn = (Game_TickPROC)dlsym(GameModule, "Game_Tick");
	Game_ShutdownFn = (Game_ShutdownPROC)dlsym(GameModule, "Game_Shutdown");

	Game_InitFn();
}

#endif /* __linux__ */
