#include <knell/kn.h>
#include <knell/log.h>
#include <knell/render.h>

#include <math.h>

LogHandle LogSysSample;

#define NUM_CIRCLE_VERTICES 70
float2 vertices[NUM_CIRCLE_VERTICES];

KN_GAME_API void Game_Init(const char* target)
{
	Log_RegisterSystem(&LogSysSample, "Sample", KN_LOG_TRACE);
	KN_TRACE(LogSysSample, "Sample loaded");

	const float radius = 50.0f;
	const float arcSize = 2 * 3.14159f / (NUM_CIRCLE_VERTICES - 1);
	for (uint32_t i=0; i < NUM_CIRCLE_VERTICES-1; ++i) {
		vertices[i] = float2_Make(50 + radius * cos(i*arcSize), 50 + radius * sinf(i*arcSize));
	}
	vertices[NUM_CIRCLE_VERTICES-1] = vertices[0];
}

KN_GAME_API void Game_Draw(void)
{
	R_StartFrame();

    rgb8 white = { 255u, 255u, 255u };
	R_DrawDebugLine(0, 0, 1024, 768, white);

    rgb8 blue = { 0u, 0u, 255u };
	R_DrawDebugLine(100, 100, 100, 200, blue);
	R_DrawDebugLine(100, 200, 200, 200, blue);
	R_DrawDebugLine(200, 200, 200, 100, blue);
	R_DrawDebugLine(100, 100, 200, 100, blue);

	R_DrawDebugLineStrip(vertices, NUM_CIRCLE_VERTICES, white);

	R_EndFrame();
}

KN_GAME_API void Game_Tick(uint64_t dt)
{
}

KN_GAME_API void Game_Shutdown()
{
}
