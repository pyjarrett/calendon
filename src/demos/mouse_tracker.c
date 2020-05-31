#include <calendon/cn.h>
#include <calendon/input.h>
#include <calendon/log.h>
#include <calendon/math2.h>
#include <calendon/render.h>
#include <calendon/time.h>

#include <math.h>
#include <calendon/ui.h>

CnLogHandle LogSysSample;

CnFloat2 position;

CN_GAME_API bool Plugin_Init(void)
{
	cnLog_RegisterSystem(&LogSysSample, "Sample", CN_LOG_TRACE);
	CN_TRACE(LogSysSample, "Sample loaded");

	position = cnFloat2_Make(300, 300);
	return true;
}

CN_GAME_API void Plugin_Draw(void)
{
	cnR_StartFrame();

	CnDimension2f rectSize = { 50, 50 };
	CnRGB8u white = { 255, 255, 255};
	cnR_DrawDebugRect(position, rectSize, white);

	cnR_EndFrame();
}

CN_GAME_API void Plugin_Tick(uint64_t dt)
{
	CN_UNUSED(dt);
	CnInput* input = cnUI_InputPoll();
	CN_ASSERT(input, "CnInput poll provided a null pointer.");
	position = cnFloat2_Make((float) input->mouse.x, (float) input->mouse.y);

}

CN_GAME_API void Plugin_Shutdown(void)
{
}
