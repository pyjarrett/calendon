#include <knell/kn.h>
#include <knell/input.h>
#include <knell/log.h>
#include <knell/math2.h>
#include <knell/render.h>
#include <knell/time.h>

#include <math.h>
#include <knell/ui.h>

LogHandle LogSysSample;

float2 position;

KN_GAME_API void Game_Init(void)
{
	Log_RegisterSystem(&LogSysSample, "Sample", KN_LOG_TRACE);
	KN_TRACE(LogSysSample, "Sample loaded");

	position = float2_Make(300, 300);
}

KN_GAME_API void Game_Draw(void)
{
	R_StartFrame();

	Dimension2f rectSize = { 50, 50 };
	rgb8 white = { 255, 255, 255};
	R_DrawDebugRect(position, rectSize, white);

	R_EndFrame();
}

KN_GAME_API void Game_Tick(uint64_t dt)
{
	KN_UNUSED(dt);
	Input* input = UI_InputPoll();
	KN_ASSERT(input, "Input poll provided a null pointer.");
	position = float2_Make((float)input->mouse.x, (float)input->mouse.y);

}

KN_GAME_API void Game_Shutdown()
{
}
