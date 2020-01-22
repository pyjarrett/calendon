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

	rgb8 white = { 255u, 255u, 255u };
	R_DrawDebugLine(0, 0, 1024, 768, white);

	R_DrawDebugLine(100, 100, 100, 200, white);
	R_DrawDebugLine(100, 200, 200, 200, white);
	R_DrawDebugLine(200, 200, 200, 100, white);
	R_DrawDebugLine(100, 100, 200, 100, white);

	R_EndFrame();
}

KN_GAME_API void Game_Tick(uint64_t dt)
{
}

KN_GAME_API void Game_Shutdown()
{
}
