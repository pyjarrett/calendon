#include "control.h"
#include "kn.h"
#include "input.h"
#include "render_hl.h"
#include "ui.h"

#include <stdio.h>

static uint64_t lastTick;

void drawFrame()
{
	rgba8i black = {0, 0, 0, 0 };
	rhl_startFrame();
	rhl_clear(black);
	rhl_endFrame();
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
	lastTick = timeNowNs();
	initWindow();
	rhl_init();
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
	const uint64_t current = timeNowNs();
	if (lastTick > current) {
		KN_FATAL_ERROR("Time went backwards");
	}

	// Prevent updating too rapidly.  Maintaining a relatively consistent
	// timestep limits stored state and prevents precision errors due to
	// extremely small dt.
	//
	// Since Knell is single-threaded, VSync will probably ensure that the
	// minimum tick size is never missed.
	const uint64_t minTickSize = msToNs(8);
	const uint64_t dt = current - lastTick;
	if (dt < minTickSize) {
		return false;
	}

	lastTick = current;

	// Ignore huge ticks, such as when resuming in the debugger.
	const uint64_t maxTickSize = secToNs(5);
	if (dt > maxTickSize) {
		printf("Skipping large tick");
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
		processWindowEvents();

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
	float4x4 s = float4x4_nonuniform_scale(1.0f, 2.0f, 3.0f);
	float4x4 t = float4x4_translate(5.0f, 6.0f, 7.0f);

	printf("S = \n");
	float4x4_debug_print(stdout, s);

	printf("T = \n");
	float4x4_debug_print(stdout, t);

	printf("T * S = \n");
	float4x4_debug_print(stdout, float4x4_multiply(t, s));

	printf("T * T = \n");
	float4x4_debug_print(stdout, float4x4_multiply(t, t));

	printf("Tt = \n");
	float4x4_debug_print(stdout, float4x4_transpose(t));
}

int main(const int argc, char* argv[])
{
	initAllSystems();
	runTest();
	//runMainLoop();
	return 0;
}
