#include "game.h"

#include <knell/kn.h>

#ifdef _WIN32

#include <knell/compat-windows.h>
typedef HMODULE knSharedLibrary;

static void SharedLibrary_Release(knSharedLibrary library)
{
	if (library) {
		FreeLibrary(library);
	}
}

static knSharedLibrary SharedLibrary_Load(const char* sharedLibraryName)
{
	return LoadLibrary(sharedLibraryName);
}

#endif /* _WIN32 */

#ifdef __linux__

#include <dlfcn.h>
typedef void* knSharedLibrary;

static void SharedLibrary_Release(knSharedLibrary library)
{
	if (library) {
		dlclose(library);
	}
}

static knSharedLibrary SharedLibrary_Load(const char* sharedLibraryName)
{
	return dlopen(sharedLibraryName,  RTLD_NOW);
}

#endif /* __linux__ */


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

	if (!Game_InitFn()) {
		KN_FATAL_ERROR("%s failed to initialize", sharedLibraryName);
	}
}
