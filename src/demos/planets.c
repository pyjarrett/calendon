/*
 * A demo showing an orbiting set of planets.
 */
#include <knell/kn.h>
#include <knell/assets.h>
#include <knell/log.h>
#include <knell/math2.h>
#include <knell/path.h>
#include <knell/render.h>
#include <knell/render-resources.h>

#include <math.h>

LogHandle LogSysSample;

#define NUM_CIRCLE_VERTICES 20
float2 circleOrigin;
float2 circleVertices[NUM_CIRCLE_VERTICES];

FontId font;
static uint64_t lastDt;

/**
 * Creates a line of points to form circle in a counter clockwise winding.
 */
void createCircle(float2* vertices, uint32_t numVertices, float radius)
{
	KN_ASSERT(vertices != NULL, "Cannot write vertices into a null pointer");
	KN_ASSERT(radius > 0.0f, "Radius must positive: %f provided", radius);
	const float arcSize = 2 * 3.14159f / (numVertices - 1);
	for (uint32_t i = 0; i < numVertices - 1; ++i) {
		vertices[i] = float2_Make(radius * cosf(i * arcSize),
			radius * sinf(i * arcSize));
	}
	vertices[numVertices - 1] = vertices[0];
}

KN_GAME_API void Game_Init(void)
{
	Log_RegisterSystem(&LogSysSample, "Sample", KN_LOG_TRACE);
	KN_TRACE(LogSysSample, "Sample loaded");

	circleOrigin = float2_Make(400.0f, 400.0f);
	createCircle(circleVertices, NUM_CIRCLE_VERTICES, 50.0f);
	for (uint32_t i = 0; i < NUM_CIRCLE_VERTICES; ++i) {
		circleVertices[i] = float2_Add(circleVertices[i], circleOrigin);
	}

	PathBuffer fontPath;
	Assets_PathBufferFor("fonts/bizcat.psf", &fontPath);
	R_CreateFont(&font);
	if (!R_LoadPSF2Font(font, fontPath.str))	{
		KN_FATAL_ERROR("Unable to load font: %s", fontPath.str);
	}
}

KN_GAME_API void Game_Draw(void)
{
	R_StartFrame();

	rgb8 red = { .r = 255, .g = 0, .b = 0 };

	R_DrawDebugLineStrip(circleVertices, NUM_CIRCLE_VERTICES, red);

	R_DrawSimpleText(font, float2_Make(300, 100), "Planets demo");

	static char frameTime[100] = "";
	lastDt = lastDt == 0 ? 1 : lastDt;
	static int fpsTick = 0;
	if (++fpsTick % 10 == 0) {
		fpsTick = 0;
		sprintf(frameTime, "FPS: %.1f", 1000000000.0f / lastDt);
	}

	R_DrawSimpleText(font, float2_Make(0, 600), frameTime);
	R_EndFrame();
}

KN_GAME_API void Game_Tick(uint64_t dt)
{
	lastDt = dt;
}

KN_GAME_API void Game_Shutdown()
{
}
