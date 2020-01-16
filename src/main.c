#include "control.h"
#include "kn.h"
#include "input.h"
#include "log.h"
#include "memory.h"
#include "time.h"
#include "render.h"
#include "ui.h"
#include "assets.h"

#include <stdio.h>

static uint64_t lastTick;

void drawFrame()
{
	R_StartFrame();
	R_EndFrame();
}

void tick(uint64_t dt)
{
	KN_UNUSED(dt);
}

/**
 * Common initialization point for all global systems.
 */
void initAllSystems()
{
	Log_Init();
	Mem_Init();

	Assets_Init("/home/paul/lab/knell/assets");

	lastTick = Time_NowNs();

	const uint32_t width = 1024;
	const uint32_t height = 768;
	UI_Init(width, height);
	R_Init(width, height);

	KN_TRACE(LogSysMain, "Systems initialized.");
}

void shutdownAllSystems()
{
	Mem_Shutdown();
	UI_Shutdown();
}

/**
 * Possibly generate a delta time for the next game update.  If the time since
 * the previous tick is too small or very large, no tick will be generated.
 * Small ticks do needless work, and large ticks might be due to resuming from
 * the debugger.
 *
 * @param[out] outDt delta time if a tick is generated (returns true), not set otherwise
 * @return true if a tick should occur
 */
bool generateTick(uint64_t* outDt)
{
	const uint64_t current = Time_NowNs();
	if (lastTick > current) {
		KN_FATAL_ERROR("Time went backwards");
	}

	// Prevent updating too rapidly.  Maintaining a relatively consistent
	// timestep limits stored state and prevents precision errors due to
	// extremely small dt.
	//
	// Since Knell is single-threaded, VSync will probably ensure that the
	// minimum tick size is never missed.
	const uint64_t minTickSize = Time_MsToNs(8);
	const uint64_t dt = current - lastTick;
	if (dt < minTickSize) {
		return false;
	}

	lastTick = current;

	// Ignore huge ticks, such as when resuming in the debugger.
	const uint64_t maxTickSize = Time_SecToNs(5);
	if (dt > maxTickSize) {
		KN_TRACE(LogSysMain, "Skipping large tick: %" PRIu64, *outDt);
		return false;
	}

	*outDt = dt;
	return true;
}

void runMainLoop()
{
	while (isRunning()) {
		// Event checking should be quick.  Always processing events prevents
		// slowness due to bursts.
		UI_ProcessWindowEvents();

		uint64_t dt;
		if (generateTick(&dt)) {
			tick(dt);
		}
		drawFrame();
	}
}

#include "math.h"
void runTest()
{
	float4x4 s = float4x4_NonUniformScale(1.0f, 2.0f, 3.0f);
	float4x4 t = float4x4_Translate(5.0f, 6.0f, 7.0f);

	float4 point = { .x = 1.0f, .y = 2.0f, .z = 3.0f, .w = 1.0f };
	float4 vector = { .x = 1.0f, .y = 2.0f, .z = 3.0f, .w = 0.0f };

	printf("S = \n");
	float4x4_DebugPrint(stdout, s);

	float4_DebugPrint(stdout, float4_Multiply(point, s));
	float4_DebugPrint(stdout, float4_Multiply(vector, s));

	printf("T = \n");
	float4x4_DebugPrint(stdout, t);

	float4_DebugPrint(stdout, float4_Multiply(point, t));
	float4_DebugPrint(stdout, float4_Multiply(vector, t));

	printf("T * S = \n");
	float4x4_DebugPrint(stdout, float4x4_Multiply(t, s));

	printf("S * T = \n");
	float4x4_DebugPrint(stdout, float4x4_Multiply(s, t));

	printf("T * T = \n");
	float4x4_DebugPrint(stdout, float4x4_Multiply(t, t));

	printf("Tt = \n");
	float4x4_DebugPrint(stdout, float4x4_Transpose(t));
}

int main(const int argc, char* argv[])
{
	initAllSystems();
	runTest();
	runMainLoop();
	shutdownAllSystems();
	return 0;
}

