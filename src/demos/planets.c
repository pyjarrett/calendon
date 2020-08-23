/*
 * A demo showing an orbiting set of planets.
 */
#include <calendon/cn.h>
#include <calendon/assets.h>
#include <calendon/log.h>
#include <calendon/math2.h>
#include <calendon/path.h>
#include <calendon/render.h>
#include <calendon/render-resources.h>
#include <calendon/time.h>

CnLogHandle LogSysSample;

CnFontId font;
static CnTime lastDt;

typedef struct {
	CnFloat2 position;
	CnFloat2 velocity;
	float mass;
	float radius;
	CnOpaqueColor color;
} CelestialBody;

#define NUM_PLANETS 4
CelestialBody bodies[NUM_PLANETS];

CN_GAME_API bool CnPlugin_Init(void)
{
	LogSysSample = cnLog_RegisterSystem("Sample");
	cnLogHandle_SetVerbosity(LogSysSample, CnLogVerbosityTrace);
	CN_TRACE(LogSysSample, "Sample loaded");

	CnPathBuffer fontPath;
	cnAssets_PathBufferFor("fonts/bizcat.psf", &fontPath);
	cnR_CreateFont(&font);
	if (!cnR_LoadPSF2Font(font, fontPath.str))	{
		CN_FATAL_ERROR("Unable to load font: %s", fontPath.str);
	}

	bodies[0].color = cnOpaqueColor_MakeRGBu8(255, 0, 0);
	bodies[0].position = cnFloat2_Make(500, 400);
	bodies[0].mass = 5000.0f;
	bodies[0].radius = 20.0f;

	bodies[1].color = cnOpaqueColor_MakeRGBu8(0, 255, 0);
	bodies[1].position = cnFloat2_Make(550, 400);
	bodies[1].mass = 5.0f;
	bodies[1].velocity = cnFloat2_Make(0.0f, 0.3f);
	bodies[1].radius = 5.0f;

	bodies[2].color = cnOpaqueColor_MakeRGBu8(150, 150, 255);
	bodies[2].position = cnFloat2_Make(600, 400);
	bodies[2].mass = 5.0f;
	bodies[2].velocity = cnFloat2_Make(0.0f, 0.18f);
	bodies[2].radius = 5.0f;

	bodies[3].color = cnOpaqueColor_MakeRGBu8(150, 150, 150);
	bodies[3].position = cnFloat2_Make(650, 400);
	bodies[3].mass = 5.0f;
	bodies[3].velocity = cnFloat2_Make(0.0f, 0.13f);
	bodies[3].radius = 5.0f;
	return true;
}

CN_GAME_API void CnPlugin_Draw(void)
{
	cnR_StartFrame();

	for (uint32_t bodyIndex = 0; bodyIndex < NUM_PLANETS; ++bodyIndex) {
		cnR_OutlineCircle(bodies[bodyIndex].position, bodies[bodyIndex].radius, bodies[bodyIndex].color, 20);
	}

	static char frameTime[100] = "";
	lastDt = cnTime_Max(cnTime_MakeMilli(1), lastDt);
	static int fpsTick = 0;
	if (++fpsTick % 10 == 0) {
		fpsTick = 0;
		cnString_Format(frameTime, 100, "FPS: %.1f", 1000.0f / cnTime_Milli(lastDt));
	}

	cnR_DrawSimpleText(font, cnFloat2_Make(0, 0), "Planets demo");
	cnR_DrawSimpleText(font, cnFloat2_Make(0, 50), frameTime);
	cnR_EndFrame();
}

CN_GAME_API void CnPlugin_Tick(CnTime dt)
{
	lastDt = dt;

	const uint64_t ms = cnTime_Milli(dt);

	const float gravitationalConstant = 0.0005f;
	const float minGravityApplication = 2.0f;

	for (uint32_t i = 0; i < NUM_PLANETS; ++i) {
		for (uint32_t j = 0; j < NUM_PLANETS; ++j) {
			if (j != i) {
				const float d2 = cnFloat2_DistanceSquared(bodies[i].position, bodies[j].position);
				const float forceGravity = gravitationalConstant * bodies[i].mass * bodies[j].mass / d2;

				// F = m * a --> da = dt * F / m
				const float accelI = forceGravity / bodies[i].mass;
				const float accelJ = forceGravity / bodies[j].mass;

				const CnFloat2 iToJ = cnFloat2_Sub(bodies[i].position, bodies[j].position);
				if (cnFloat2_LengthSquared(iToJ) < minGravityApplication) {
					continue;
				}
				const CnFloat2 iToJNormalized = cnFloat2_Normalize(iToJ);

				bodies[i].velocity = cnFloat2_Add(bodies[i].velocity,
												  cnFloat2_Multiply(iToJNormalized, (float) ms * -accelI));
				bodies[j].velocity = cnFloat2_Add(bodies[j].velocity,
												  cnFloat2_Multiply(iToJNormalized, (float) ms * accelJ));
			}
		}
	}

	for (uint32_t i = 0; i < NUM_PLANETS; ++i) {
		bodies[i].position = cnFloat2_Add(bodies[i].position, cnFloat2_Multiply(bodies[i].velocity, (float)ms));
	}
}

CN_GAME_API void CnPlugin_Shutdown(void)
{
}
