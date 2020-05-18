/*
 * Working with animation loops.
 */
#include <knell/kn.h>
#include <knell/anim-loop.h>
#include <knell/log.h>
#include <knell/math2.h>
#include <knell/render.h>
#include <knell/time.h>

LogHandle LogSysSample;

#define SAMPLE_POINTS 4
AnimationLoopCursor sampleCursor;
AnimationLoop sampleLoop;
float2 points[SAMPLE_POINTS];

KN_GAME_API bool Game_Init(void)
{
	Log_RegisterSystem(&LogSysSample, "Sample", KN_LOG_TRACE);
	KN_TRACE(LogSysSample, "Sample loaded");

	KN_TRACE(LogSysSample, "Animation loop size:        %zu bytes", sizeof(AnimationLoop));
	KN_TRACE(LogSysSample, "Animation loop cursor size: %zu bytes", sizeof(AnimationLoopCursor));

	sampleLoop.numStates = SAMPLE_POINTS;
	sampleLoop.elapsed[0] = Time_MsToNs(400);
	sampleLoop.elapsed[1] = Time_MsToNs(1000);
	sampleLoop.elapsed[2] = Time_MsToNs(2000);
	sampleLoop.elapsed[3] = Time_MsToNs(500);

	points[0] = float2_Make(200, 200);
	points[1] = float2_Make(400, 200);
	points[2] = float2_Make(400, 400);
	points[3] = float2_Make(200, 400);
	return true;
}

KN_GAME_API void Game_Draw(void)
{
	R_StartFrame();

	Dimension2f rectSize = { 50, 50 };
	rgb8 white = { 255, 255, 255 };

	float2 animatedPosition = float2_Lerp(points[sampleCursor.current],
		points[AnimLoop_NextState(&sampleLoop, sampleCursor.current)],
		AnimLoop_CalcAlpha(&sampleLoop, &sampleCursor));
	R_DrawDebugRect(animatedPosition, rectSize, white);

	R_EndFrame();
}

KN_GAME_API void Game_Tick(uint64_t dt)
{
	AnimLoop_Tick(&sampleLoop, &sampleCursor, dt);
}

KN_GAME_API void Game_Shutdown(void)
{
}
