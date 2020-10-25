/*
 * Working with animation loops.
 */
#include <calendon/cn.h>
#include <calendon/anim-loop.h>
#include <calendon/log.h>
#include <calendon/math2.h>
#include <calendon/render.h>
#include <calendon/time.h>

CnLogHandle LogSysSample;

#define SAMPLE_POINTS 4
CnAnimationLoopCursor sampleCursor;
CnAnimationLoop sampleLoop;
CnFloat2 points[SAMPLE_POINTS];

CN_GAME_API bool Demo_Init(void)
{
	LogSysSample = cnLog_RegisterSystem("Sample");
	cnLog_SetVerbosity(LogSysSample, CnLogVerbosityTrace);
	CN_TRACE(LogSysSample, "Sample loaded");

	CN_TRACE(LogSysSample, "Animation loop size:        %zu bytes", sizeof(CnAnimationLoop));
	CN_TRACE(LogSysSample, "Animation loop cursor size: %zu bytes", sizeof(CnAnimationLoopCursor));

	sampleLoop.numStates = SAMPLE_POINTS;
	sampleLoop.elapsed[0] = cnTime_MakeMilli(400);
	sampleLoop.elapsed[1] = cnTime_MakeMilli(1000);
	sampleLoop.elapsed[2] = cnTime_MakeMilli(2000);
	sampleLoop.elapsed[3] = cnTime_MakeMilli(500);

	points[0] = cnFloat2_Make(200, 200);
	points[1] = cnFloat2_Make(400, 200);
	points[2] = cnFloat2_Make(400, 400);
	points[3] = cnFloat2_Make(200, 400);
	return true;
}

CN_GAME_API void Demo_Draw(CnFrameEvent* event)
{
	CN_UNUSED(event);
	cnR_StartFrame();

	const CnDimension2f rectSize = { 50, 50 };
	const CnOpaqueColor white = cnOpaqueColor_MakeRGBu8(255, 255, 255);

	const CnFloat2 animatedPosition = cnFloat2_Lerp(points[sampleCursor.current],
		points[AnimLoop_NextState(&sampleLoop, sampleCursor.current)],
		cnAnimLoop_CalcAlpha(&sampleLoop, &sampleCursor));
	cnR_DrawDebugRect(animatedPosition, rectSize, white);

	cnR_EndFrame();
}

CN_GAME_API void Demo_Tick(CnFrameEvent* event)
{
	CN_ASSERT_PTR(event);
	cnAnimLoop_Tick(&sampleLoop, &sampleCursor, event->dt);
}
