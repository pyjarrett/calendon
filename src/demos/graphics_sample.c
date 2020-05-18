/*
 * Working with animation loops.
 */
#include <knell/kn.h>
#include <knell/anim-loop.h>
#include <knell/assets.h>
#include <knell/log.h>
#include <knell/math2.h>
#include <knell/path.h>
#include <knell/render.h>
#include <knell/render-resources.h>
#include <knell/time.h>

#include <math.h>

LogHandle LogSysSample;

AnimationLoopCursor sampleCursor;
AnimationLoop sampleLoop;

#define SPRITE_ANIMATION_FRAMES 3
SpriteId spriteFrames[SPRITE_ANIMATION_FRAMES];

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

KN_GAME_API bool Game_Init(void)
{
	Log_RegisterSystem(&LogSysSample, "Sample", KN_LOG_TRACE);
	KN_TRACE(LogSysSample, "Sample loaded");

	KN_TRACE(LogSysSample, "Animation loop size:        %zu bytes", sizeof(AnimationLoop));
	KN_TRACE(LogSysSample, "Animation loop cursor size: %zu bytes", sizeof(AnimationLoopCursor));

	sampleLoop.numStates = SPRITE_ANIMATION_FRAMES;
	sampleLoop.elapsed[0] = Time_MsToNs(150);
	sampleLoop.elapsed[1] = Time_MsToNs(150);
	sampleLoop.elapsed[2] = Time_MsToNs(150);

	R_CreateSprite(&spriteFrames[0]);
	R_CreateSprite(&spriteFrames[1]);
	R_CreateSprite(&spriteFrames[2]);

	const char* frameFilenames[] = {
		"sprites/stick_person.png",
		"sprites/stick_person2.png",
		"sprites/stick_person3.png"
	};

	for (uint32_t i = 0; i < 3; ++i) {
		PathBuffer path;
		Assets_PathBufferFor(frameFilenames[i], &path);
		R_LoadSprite(spriteFrames[i], path.str);
	}

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
	return true;
}

KN_GAME_API void Game_Draw(void)
{
	R_StartFrame();

	R_DrawDebugFullScreenRect();

	float2 position = float2_Make(100, 100);
	Dimension2f size = { .width = 100.0f, .height = 100.0f };
	R_DrawSprite(spriteFrames[sampleCursor.current], position, size);

	rgb8 red = { .r = 255, .g = 0, .b = 0 };
	rgb8 green = { .r = 0, .g = 255, .b = 0 };
	rgb8 blue = { .r = 0, .g = 0, .b = 255 };

	R_DrawDebugLineStrip(circleVertices, NUM_CIRCLE_VERTICES, red);
	static int step = 0;
	--step;
	if (step < 0) step = NUM_CIRCLE_VERTICES - 2;
	R_DrawDebugLine(circleOrigin.x, circleOrigin.y, circleVertices[step].x,
		circleVertices[step].y, green);

	float2 rectPosition = float2_Make(200, 100);
	Dimension2f rectSize = { .width = 100.0f, .height = 100.0f };
	R_DrawDebugRect(rectPosition, rectSize, green);

	R_DrawSimpleText(font, float2_Make(300, 100), "Hello, Paul!\xe2\x86\x93→\xe2\x86\x92");
	R_DrawSimpleText(font, float2_Make(100, 500), "«café, caffè» ™ © Â ←");

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
	AnimLoop_Tick(&sampleLoop, &sampleCursor, dt);
	lastDt = dt;
}

KN_GAME_API void Game_Shutdown(void)
{
}
