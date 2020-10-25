#include <calendon/cn.h>
#include <calendon/input-keyset.h>
#include <calendon/log.h>
#include <calendon/math2.h>
#include <calendon/render.h>
#include <calendon/time.h>

#include <math.h>
#include <calendon/ui.h>

CnLogHandle LogSysSample;

CnFloat2 position;

CN_GAME_API bool Demo_Init(void)
{
	LogSysSample = cnLog_RegisterSystem("Sample");
	cnLog_SetVerbosity(LogSysSample, CnLogVerbosityTrace);
	CN_TRACE(LogSysSample, "Sample loaded");

	position = cnFloat2_Make(300, 300);
	return true;
}

CN_GAME_API void Demo_Draw(CnFrameEvent* event)
{
	CN_UNUSED(event);
	cnR_StartFrame();

	CnDimension2f rectSize = { 50, 50 };
	const CnOpaqueColor white = cnOpaqueColor_MakeRGBu8(255, 255, 255);
	cnR_DrawDebugRect(position, rectSize, white);

	cnR_EndFrame();
}

CN_GAME_API void Demo_Tick(CnFrameEvent* event)
{
	CN_UNUSED(event);
	CnInput* input = cnInput_Poll();
	CN_ASSERT(input, "CnInput poll provided a null pointer.");
	position = cnFloat2_Make((float) input->mouse.x, (float) input->mouse.y);

}
