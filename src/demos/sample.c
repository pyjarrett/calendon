#include <calendon/cn.h>
#include <calendon/log.h>
#include <calendon/render.h>
#include <calendon/time.h>

#include <math.h>

CnLogHandle LogSysSample;

#define NUM_CIRCLE_VERTICES 70
CnFloat2 vertices[NUM_CIRCLE_VERTICES];

CN_GAME_API bool Demo_Init(void)
{
	LogSysSample = cnLog_RegisterSystem("Sample");
	cnLog_SetVerbosity(LogSysSample, CnLogVerbosityTrace);
	CN_TRACE(LogSysSample, "Sample loaded");

	const float radius = 50.0f;
	const float arcSize = 2 * 3.14159f / (NUM_CIRCLE_VERTICES - 1);
	for (uint32_t i=0; i < NUM_CIRCLE_VERTICES-1; ++i) {
		vertices[i] = cnFloat2_Make(50 + radius * cosf(i * arcSize), 50 + radius * sinf(i * arcSize));
	}
	vertices[NUM_CIRCLE_VERTICES-1] = vertices[0];
	return true;
}

CN_GAME_API void Demo_Draw(CnFrameEvent* event)
{
	CN_UNUSED(event);
	cnR_StartFrame();

    const CnOpaqueColor white = cnOpaqueColor_MakeRGBu8(255, 255, 255);
	cnR_DrawDebugLine(0, 0, 1024, 768, white);

    const CnOpaqueColor blue = cnOpaqueColor_MakeRGBu8(0, 0, 255);
	cnR_DrawDebugLine(100, 100, 100, 200, blue);
	cnR_DrawDebugLine(100, 200, 200, 200, blue);
	cnR_DrawDebugLine(200, 200, 200, 100, blue);
	cnR_DrawDebugLine(100, 100, 200, 100, blue);

	cnR_DrawDebugLineStrip(vertices, NUM_CIRCLE_VERTICES, white);

	cnR_EndFrame();
}

CN_GAME_API void Demo_Tick(CnFrameEvent* event)
{
	CN_UNUSED(event);
}
