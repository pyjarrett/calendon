#include <knell/kn.h>
#include <knell/log.h>
#include <knell/render.h>

LogHandle LogSysSample;

KN_GAME_API void Game_Init(const char* target)
{
	Log_RegisterSystem(&LogSysSample, "Sample", KN_LOG_TRACE);
	KN_TRACE(LogSysSample, "Sample loaded");
}

KN_GAME_API void Game_Draw(void)
{
	R_StartFrame();
	R_DrawDebugFullScreenRect();
	R_EndFrame();
}

KN_GAME_API void Game_Tick(uint64_t dt)
{
}

KN_GAME_API void Game_Shutdown()
{
}
