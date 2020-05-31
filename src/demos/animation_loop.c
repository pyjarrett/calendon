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

CN_GAME_API bool CnPlugin_Init(void)
{
	cnLog_RegisterSystem(&LogSysSample, "Sample", CN_LOG_TRACE);
	CN_TRACE(LogSysSample, "Sample loaded");

	CN_TRACE(LogSysSample, "Animation loop size:        %zu bytes", sizeof(CnAnimationLoop));
	CN_TRACE(LogSysSample, "Animation loop cursor size: %zu bytes", sizeof(CnAnimationLoopCursor));

	sampleLoop.numStates = SAMPLE_POINTS;
	sampleLoop.elapsed[0] = cnTime_MsToNs(400);
	sampleLoop.elapsed[1] = cnTime_MsToNs(1000);
	sampleLoop.elapsed[2] = cnTime_MsToNs(2000);
	sampleLoop.elapsed[3] = cnTime_MsToNs(500);

	points[0] = cnFloat2_Make(200, 200);
	points[1] = cnFloat2_Make(400, 200);
	points[2] = cnFloat2_Make(400, 400);
	points[3] = cnFloat2_Make(200, 400);
	return true;
}

CN_GAME_API void CnPlugin_Draw(void)
{
	cnR_StartFrame();

	CnDimension2f rectSize = { 50, 50 };
	CnRGB8u white = { 255, 255, 255 };

	CnFloat2 animatedPosition = cnFloat2_Lerp(points[sampleCursor.current],
											  points[AnimLoop_NextState(&sampleLoop, sampleCursor.current)],
											  cnAnimLoop_CalcAlpha(&sampleLoop, &sampleCursor));
	cnR_DrawDebugRect(animatedPosition, rectSize, white);

	cnR_EndFrame();
}

CN_GAME_API void CnPlugin_Tick(uint64_t dt)
{
	cnAnimLoop_Tick(&sampleLoop, &sampleCursor, dt);
}

CN_GAME_API void CnPlugin_Shutdown(void)
{
}
