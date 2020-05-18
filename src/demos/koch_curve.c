#include <knell/kn.h>
#include <knell/log.h>
#include <knell/math2.h>
#include <knell/render.h>
#include <knell/time.h>

#include <math.h>

LogHandle LogSysSample;

uint32_t numCurrentPoints = 0;
uint64_t timeBeforeStep;
uint64_t currentTime;

#define MAX_POINTS 128
float2 points[2][MAX_POINTS];
uint32_t currentBuffer = 0;

void reset(void)
{
	currentBuffer = 0;
	points[currentBuffer][0] = float2_Make(200, 200);
	points[currentBuffer][1] = float2_Make(600, 200);
	numCurrentPoints = 2;
}

void step(void)
{
	// Subdivide each line segment in turn, each line of 2 points changes into
	// 5 points.
	if (numCurrentPoints * 4 + 1 < MAX_POINTS) {
		float2* c = points[currentBuffer];
		float2* p = points[(currentBuffer + 1) % 2];

        KN_ASSERT(c != p, "Must read/write to separate vertex arrays");
		uint32_t numNewPoints = 0;

		// Start at 1 so the previous point always exists.
		for (uint32_t i = 1; i < numCurrentPoints; ++i) {
			const float2 start = c[i - 1];
			const float2 end = c[i];

			const float distance = float2_Length(float2_Sub(end, start));
			const PlanarAngle lToR = float2_DirectionBetween(start, end);

			const float2 first = float2_Add(start, float2_FromPolar(distance / 3.0f, lToR));
			const float2 second = float2_Add(float2_Midpoint(start, end), float2_FromPolar(sqrtf(3.0f) * distance / 3.0f / 2.0f, Angle_Add(Angle_InDegrees(90.0f), lToR)));
			const float2 third = float2_Add(start, float2_FromPolar(distance * 2.0f / 3.0f, lToR));

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

KN_GAME_API bool Game_Init(void)
{
	Log_RegisterSystem(&LogSysSample, "Sample", KN_LOG_TRACE);
	KN_TRACE(LogSysSample, "Sample loaded");

	reset();
    timeBeforeStep = Time_SecToNs(3);
    currentTime = 0;
    return true;
}

KN_GAME_API void Game_Draw(void)
{
	R_StartFrame();
	rgb8 white = { 255u, 255u, 255u };
	R_DrawDebugLineStrip(points[currentBuffer], numCurrentPoints, white);
	R_EndFrame();
}

KN_GAME_API void Game_Tick(uint64_t dt)
{
	currentTime += dt;
	if (currentTime > timeBeforeStep) {
		step();
		currentTime -= timeBeforeStep;
	}
}

KN_GAME_API void Game_Shutdown(void)
{
}
