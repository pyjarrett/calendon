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
#include <knell/time.h>

LogHandle LogSysSample;

FontId font;
static uint64_t lastDt;

typedef struct {
	float2 position;
	float2 velocity;
	float mass;
	float radius;
	rgb8 color;
} CelestialBody;

#define NUM_PLANETS 4
CelestialBody bodies[NUM_PLANETS];

KN_GAME_API bool Plugin_Init(void)
{
	Log_RegisterSystem(&LogSysSample, "Sample", KN_LOG_TRACE);
	KN_TRACE(LogSysSample, "Sample loaded");

	PathBuffer fontPath;
	Assets_PathBufferFor("fonts/bizcat.psf", &fontPath);
	R_CreateFont(&font);
	if (!R_LoadPSF2Font(font, fontPath.str))	{
		KN_FATAL_ERROR("Unable to load font: %s", fontPath.str);
	}

	bodies[0].color = (rgb8){ .r = 255, .g = 0, .b = 0 };
	bodies[0].position = float2_Make(500, 400);
	bodies[0].mass = 5000.0f;
	bodies[0].radius = 20.0f;

	bodies[1].color = (rgb8){ .r = 0, .g = 255, .b = 0 };
	bodies[1].position = float2_Make(550, 400);
	bodies[1].mass = 5.0f;
	bodies[1].velocity = float2_Make(0.0f, 0.3f);
	bodies[1].radius = 5.0f;

	bodies[2].color = (rgb8){ .r = 150, .g = 150, .b = 255 };
	bodies[2].position = float2_Make(600, 400);
	bodies[2].mass = 5.0f;
	bodies[2].velocity = float2_Make(0.0f, 0.18f);
	bodies[2].radius = 5.0f;

	bodies[3].color = (rgb8){ .r = 150, .g = 150, .b = 150 };
	bodies[3].position = float2_Make(650, 400);
	bodies[3].mass = 5.0f;
	bodies[3].velocity = float2_Make(0.0f, 0.13f);
	bodies[3].radius = 5.0f;
	return true;
}

KN_GAME_API void Plugin_Draw(void)
{
	R_StartFrame();

	for (uint32_t bodyIndex = 0; bodyIndex < NUM_PLANETS; ++bodyIndex) {
		R_OutlineCircle(bodies[bodyIndex].position, bodies[bodyIndex].radius, bodies[bodyIndex].color, 20);
	}

	static char frameTime[100] = "";
	lastDt = lastDt == 0 ? 1 : lastDt;
	static int fpsTick = 0;
	if (++fpsTick % 10 == 0) {
		fpsTick = 0;
		sprintf(frameTime, "FPS: %.1f", 1000000000.0f / lastDt);
	}

	R_DrawSimpleText(font, float2_Make(0, 0), "Planets demo");
	R_DrawSimpleText(font, float2_Make(0, 50), frameTime);
	R_EndFrame();
}

KN_GAME_API void Plugin_Tick(uint64_t dt)
{
	lastDt = dt;

	const uint64_t ms = Time_NsToMs(dt);

	const float gravitationalConstant = 0.0005f;
	const float minGravityApplication = 2.0f;

	for (uint32_t i = 0; i < NUM_PLANETS; ++i) {
		for (uint32_t j = 0; j < NUM_PLANETS; ++j) {
			if (j != i) {
				const float d2 = float2_DistanceSquared(bodies[i].position, bodies[j].position);
				const float forceGravity = gravitationalConstant * bodies[i].mass * bodies[j].mass / d2;

				// F = m * a --> da = dt * F / m
				const float accelI = forceGravity / bodies[i].mass;
				const float accelJ = forceGravity / bodies[j].mass;

				const float2 iToJ = float2_Sub(bodies[i].position, bodies[j].position);
				if (float2_LengthSquared(iToJ) < minGravityApplication) {
					continue;
				}
				const float2 iToJNormalized = float2_Normalize(iToJ);

				bodies[i].velocity = float2_Add(bodies[i].velocity, float2_Multiply(iToJNormalized, (float)ms * -accelI));
				bodies[j].velocity = float2_Add(bodies[j].velocity, float2_Multiply(iToJNormalized, (float)ms * accelJ));
			}
		}
	}

	for (uint32_t i = 0; i < NUM_PLANETS; ++i) {
		bodies[i].position = float2_Add(bodies[i].position, float2_Multiply(bodies[i].velocity, ms));
	}
}

KN_GAME_API void Plugin_Shutdown(void)
{
}
