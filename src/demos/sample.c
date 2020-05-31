#include <calendon/cn.h>
#include <calendon/log.h>
#include <calendon/render.h>

#include <math.h>

CnLogHandle LogSysSample;

#define NUM_CIRCLE_VERTICES 70
CnFloat2 vertices[NUM_CIRCLE_VERTICES];

CN_GAME_API bool CnPlugin_Init(void)
{
	cnLog_RegisterSystem(&LogSysSample, "Sample", CN_LOG_TRACE);
	CN_TRACE(LogSysSample, "Sample loaded");

	const float radius = 50.0f;
	const float arcSize = 2 * 3.14159f / (NUM_CIRCLE_VERTICES - 1);
	for (uint32_t i=0; i < NUM_CIRCLE_VERTICES-1; ++i) {
		vertices[i] = cnFloat2_Make(50 + radius * cosf(i * arcSize), 50 + radius * sinf(i * arcSize));
	}
	vertices[NUM_CIRCLE_VERTICES-1] = vertices[0];
	return true;
}

CN_GAME_API void CnPlugin_Draw(void)
{
	cnR_StartFrame();

    CnRGB8u white = { 255u, 255u, 255u };
	cnR_DrawDebugLine(0, 0, 1024, 768, white);

    CnRGB8u blue = { 0u, 0u, 255u };
	cnR_DrawDebugLine(100, 100, 100, 200, blue);
	cnR_DrawDebugLine(100, 200, 200, 200, blue);
	cnR_DrawDebugLine(200, 200, 200, 100, blue);
	cnR_DrawDebugLine(100, 100, 200, 100, blue);

	cnR_DrawDebugLineStrip(vertices, NUM_CIRCLE_VERTICES, white);

	cnR_EndFrame();
}

CN_GAME_API void CnPlugin_Tick(uint64_t dt)
{
}

CN_GAME_API void CnPlugin_Shutdown(void)
{
}
