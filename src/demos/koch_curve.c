#include <calendon/cn.h>
#include <calendon/log.h>
#include <calendon/math2.h>
#include <calendon/render.h>
#include <calendon/time.h>

#include <math.h>

CnLogHandle LogSysSample;

uint32_t numCurrentPoints = 0;
CnTime timeBeforeStep;
CnTime currentTime;

#define MAX_POINTS 128
CnFloat2 points[2][MAX_POINTS];
uint32_t currentBuffer = 0;

void reset(void)
{
	currentBuffer = 0;
	points[currentBuffer][0] = cnFloat2_Make(200, 200);
	points[currentBuffer][1] = cnFloat2_Make(600, 200);
	numCurrentPoints = 2;
}

void step(void)
{
	// Subdivide each line segment in turn, each line of 2 points changes into
	// 5 points.
	if (numCurrentPoints * 4 + 1 < MAX_POINTS) {
		CnFloat2* c = points[currentBuffer];
		CnFloat2* p = points[(currentBuffer + 1) % 2];

        CN_ASSERT(c != p, "Must read/write to separate vertex arrays");
		uint32_t numNewPoints = 0;

		// Start at 1 so the previous point always exists.
		for (uint32_t i = 1; i < numCurrentPoints; ++i) {
			const CnFloat2 start = c[i - 1];
			const CnFloat2 end = c[i];

			const float distance = cnFloat2_Length(cnFloat2_Sub(end, start));
			const CnPlanarAngle lToR = cnFloat2_DirectionBetween(start, end);

			const CnFloat2 first = cnFloat2_Add(start, cnFloat2_FromPolar(distance / 3.0f, lToR));
			const CnFloat2 second = cnFloat2_Add(cnFloat2_Midpoint(start, end),
												 cnFloat2_FromPolar(sqrtf(3.0f) * distance / 3.0f / 2.0f,
													 cnPlanarAngle_Add(
														 cnPlanarAngle_MakeDegrees(90.0f),
														 lToR)));
			const CnFloat2 third = cnFloat2_Add(start, cnFloat2_FromPolar(distance * 2.0f / 3.0f, lToR));

			*p++ = start;
			*p++ = first;
			*p++ = second;
			*p++ = third;
			numNewPoints += 4;
		}
        *p++ = c[numCurrentPoints-1];
		++numNewPoints;
		currentBuffer = ((currentBuffer + 1) % 2);
		numCurrentPoints = numNewPoints;
	}
	else {
		reset();
	}
}

CN_GAME_API bool CnPlugin_Init(void)
{
	cnLog_RegisterSystem(&LogSysSample, "Sample", CnLogVerbosityTrace);
	CN_TRACE(LogSysSample, "Sample loaded");

	reset();
    timeBeforeStep = cnTime_MakeMilli(3000);
    currentTime = cnTime_MakeZero();
    return true;
}

CN_GAME_API void CnPlugin_Draw(void)
{
	cnR_StartFrame();
	const CnOpaqueColor white = cnOpaqueColor_MakeRGBu8(255, 255, 255);
	cnR_DrawDebugLineStrip(points[currentBuffer], numCurrentPoints, white);
	cnR_EndFrame();
}

CN_GAME_API void CnPlugin_Tick(CnTime dt)
{
	currentTime = cnTime_Add(currentTime, dt);
	if (cnTime_LessThan(timeBeforeStep, currentTime)) {
		step();
		currentTime = cnTime_SubtractMonotonic(currentTime, timeBeforeStep);
	}
}

CN_GAME_API void CnPlugin_Shutdown(void)
{
}
