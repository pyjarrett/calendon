#include <knell/kn.h>
#include <knell/log.h>
#include <knell/render.h>

LogHandle LogSysSample;

#define KN_EXPORT __declspec(dllexport)

KN_EXPORT void Game_Init(const char* target)
{
	Log_RegisterSystem(&LogSysSample, "Sample", KN_LOG_TRACE);
	KN_TRACE(LogSysSample, "Sample loaded");
}

KN_EXPORT void Game_Draw(void)
{
	R_StartFrame();
	R_EndFrame();
}

KN_EXPORT void Game_Tick(uint64_t dt)
{
}

KN_EXPORT void Game_Shutdown()
{
}
