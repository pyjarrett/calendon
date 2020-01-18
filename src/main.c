#include "knell/control.h"
#include "knell/env.h"
#include "knell/kn.h"
#include "knell/game.h"
#include "knell/input.h"
#include "knell/log.h"
#include "knell/memory.h"
#include "knell/time.h"
#include "knell/render.h"
#include "knell/ui.h"
#include "knell/assets.h"
#include "knell/process.h"

#include <stdio.h>

static uint64_t frames = 0;
static uint64_t lastTick;

void Main_DescribeEnv(void)
{
	char buffer[1024];
	Env_CurrentWorkingDirectory(buffer, 1024);
	KN_TRACE(LogSysMain, "CWD: '%s'", buffer);
}

/**
 * Common initialization point for all global systems.
 */
void Main_InitAllSystems()
{
	Log_Init();
	Mem_Init();
	Time_Init();

#ifdef _WIN32
    Assets_Init("C:/workshop/knell/assets");
    Game_Load("C:/workshop/knell/cmake-build-debug/src/demos/sample.dll");
#else
    Assets_Init("/home/paul/lab/knell/assets");
	Game_Load("/home/paul/lab/knell/cmake-build-debug/src/demos/libsample.so");
#endif

	lastTick = Time_NowNs();

	const uint32_t width = 1024;
	const uint32_t height = 768;
	UI_Init(width, height);
	R_Init(width, height);

	KN_TRACE(LogSysMain, "Systems initialized.");

#ifdef _WIN32
	// TODO: This should be hidden unless doing an "diagnostic-startup-crash" or some other special behavior.
	Proc_PrintLoadedDLLs();
#endif
}

void Main_Shutdown()
{
	Game_ShutdownFn();
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
bool Main_GenerateTick(uint64_t* outDt)
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

void Main_Loop()
{
	while (isRunning()) {
		// Event checking should be quick.  Always processing events prevents
		// slowness due to bursts.
		UI_ProcessWindowEvents();

		uint64_t dt;
		if (Main_GenerateTick(&dt)) {
			Game_TickFn(dt);
			++frames;
		}
		Game_DrawFn();

		if (frames == 500) {
#ifdef _WIN32
			Game_Load("C:/workshop/knell/cmake-build-debug/src/demos/sample2.dll");
#else
			Game_Load("/home/paul/lab/knell/cmake-build-debug/src/demos/libsample2.so");
#endif
		}
	}
}

int main(int argc, char* argv[])
{
	KN_UNUSED(argc);
	KN_UNUSED(argv);

	Main_DescribeEnv();
	Main_InitAllSystems();
	Main_Loop();
	Main_Shutdown();
	return 0;
}

