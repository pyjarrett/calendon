#include "game.h"

#include "kn.h"
#include "process.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
HMODULE GameModule;

void Game_Load(const char* sharedLibraryName)
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
	Game_DrawFn = (Game_DrawPROC)GetProcAddress(GameModule, "Game_Draw");
	Game_TickFn = (Game_TickPROC)GetProcAddress(GameModule, "Game_Tick");
	Game_ShutdownFn = (Game_ShutdownPROC)GetProcAddress(GameModule, "Game_Shutdown");

	Game_InitFn();
}
