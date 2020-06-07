#include "main.h"

#include <calendon/assets.h>
#include <calendon/assets-fileio.h>
#include <calendon/control.h>
#include <calendon/crash.h>
#include <calendon/env.h>
#include <calendon/input.h>
#include <calendon/log.h>
#include <calendon/main-config.h>
#include <calendon/memory.h>
#include <calendon/plugin.h>
#ifdef _WIN32
#include <calendon/process.h>
#endif
#include <calendon/path.h>
#include <calendon/tick-limits.h>
#include <calendon/time.h>
#include <calendon/render.h>
#include <calendon/ui.h>

#include <errno.h>
#include <stdio.h>
#include <time.h>

static uint64_t lastTick;
static CnPlugin Payload;

static CnPlugin_InitFn Main_Init;
static CnPlugin_TickFn Main_Tick;
static CnPlugin_DrawFn Main_Draw;
static CnPlugin_ShutdownFn Main_Shutdown;

void cnMain_DescribeEnv(void)
{
	char buffer[1024];
	cnEnv_CurrentWorkingDirectory(buffer, 1024);
	CN_TRACE(LogSysMain, "CWD: '%s'", buffer);
}

void cnMain_RegisterPayload(CnPlugin* payload)
{
	CN_ASSERT(payload, "Cannot register a null payload.");

	if (!payload->init) CN_FATAL_ERROR("CnPlugin_Init function missing in payload.");
	if (!payload->draw) CN_FATAL_ERROR("CnPlugin_DrawFn function missing in payload.");
	if (!payload->tick) CN_FATAL_ERROR("CnPlugin_TickFn function missing in payload.");
	if (!payload->shutdown) CN_FATAL_ERROR("CnPlugin_ShutdownFn function missing in payload.");

	Main_Init = payload->init;
	Main_Tick = payload->tick;
	Main_Draw = payload->draw;
	Main_Shutdown = payload->shutdown;
}

void cnMain_LoadPayloadFromFile(const char* sharedLibraryName)
{
	CN_ASSERT(sharedLibraryName, "Cannot use a null shared library name to load a payload.");
	uint64_t gameLibModified;
	if (!cnAssets_LastModifiedTime(sharedLibraryName, &gameLibModified)) {
		CN_FATAL_ERROR("Unable to determine last modified time of '%s'", sharedLibraryName);
	}

	struct tm *lt = localtime((time_t*)&gameLibModified);
	char timeBuffer[80];
	strftime(timeBuffer, sizeof(timeBuffer), "%c", lt);
	CN_TRACE(LogSysMain, "Last modified time: %s", timeBuffer);

	if (Main_Shutdown) {
		Main_Shutdown();
	}

	cnSharedLibrary_Release(Payload.sharedLibrary);
	Payload.sharedLibrary = cnSharedLibrary_Load(sharedLibraryName);

	if (!cnPlugin_LoadFromFile(&Payload, sharedLibraryName)) {
		CN_FATAL_ERROR("Unable to load game module: %s", sharedLibraryName);
	}

	cnMain_RegisterPayload(&Payload);

	if (!Payload.init()) {
		CN_FATAL_ERROR("%s failed to initialize", sharedLibraryName);
	}
}

/**
 * Common initialization point for all global systems.
 */
void cnMain_InitAllSystems(CnMainConfig* config)
{
	CN_ASSERT(config, "Cannot initialize using a null CnMainConfig.");
	cnLog_Init();
	cnCrash_Init();
	cnMem_Init();
	cnTime_Init();

	if (strlen(config->assetDirPath.str) != 0) {
		cnAssets_Init(config->assetDirPath.str);
	}
	else {
		// If no asset directory was provided, look for `$CALENDON_HOME/assets`, or
		// for an `assets/` directory in the working directory.
		CnPathBuffer defaultAssetDir;
		if (!cnEnv_DefaultCalendonHome(&defaultAssetDir)) {
			CN_FATAL_ERROR("Unable to determine the current Calendon home directory.");
		}
		if (!cnPathBuffer_Join(&defaultAssetDir, "assets")) {
			CN_FATAL_ERROR("Unable to assemble a default asset directory name.");
		}
		cnAssets_Init(defaultAssetDir.str);
	}

	const uint32_t width = 1024;
	const uint32_t height = 768;
	cnUI_Init(width, height);
	cnR_Init(width, height);

	if (!cnMainConfig_IsHosted(config)) {
		if (!cnPath_IsFile(config->gameLibPath.str)) {
			CN_FATAL_ERROR("Cannot load game. '%s' is not a game library.", config->gameLibPath.str);
		}

		const char* gameLib = config->gameLibPath.str;
		if (gameLib) {
			cnMain_LoadPayloadFromFile(gameLib);
		}
	}
	else {
		cnMain_RegisterPayload(&config->payload);
	}

	if (Main_Init) {
		Main_Init();
	}
	if (!Main_Draw) CN_FATAL_ERROR("Draw function missing. Write a Main_Draw(void) function.");
	if (!Main_Tick) CN_FATAL_ERROR("Update function missing. Write a Main_Tick(void) function.");

	cnMain_SetTickLimit(config->tickLimit);

	lastTick = cnTime_NowNs();

	CN_TRACE(LogSysMain, "Systems initialized.");

#ifdef _WIN32
	// TODO: This should be hidden unless doing an "diagnostic-startup-crash" or some other special behavior.
	//cnProc_PrintLoadedDLLs();
#endif
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
bool cnMain_GenerateTick(uint64_t* outDt)
{
	const uint64_t current = cnTime_NowNs();
	if (lastTick > current) {
		CN_FATAL_ERROR("Time went backwards");
	}

	// Prevent updating too rapidly.  Maintaining a relatively consistent
	// timestep limits stored state and prevents precision errors due to
	// extremely small dt.
	//
	// Since Calendon is single-threaded, VSync will probably ensure that the
	// minimum tick size is never missed.
	const uint64_t minTickSize = cnTime_MsToNs(8);
	const uint64_t dt = current - lastTick;
	if (dt < minTickSize) {
		return false;
	}

	lastTick = current;

	// Ignore huge ticks, such as when resuming in the debugger.
	const uint64_t maxTickSize = cnTime_SecToNs(5);
	if (dt > maxTickSize) {
		CN_TRACE(LogSysMain, "Skipping large tick: %" PRIu64, *outDt);
		return false;
	}

	*outDt = dt;
	return true;
}

void cnMain_Init(CnMainConfig* config)
{
	CN_ASSERT(config, "Cannot initialize a driver with a null CnMainConfig.");
	cnMain_DescribeEnv();
	cnMain_InitAllSystems(config);
}

/**
 * The big loop which processes events, ticks and draws until the game is ready
 * to shut down.
 */
void cnMain_Loop(void)
{
	CN_ASSERT(Main_Tick, "Tick function not defined.");
	CN_ASSERT(Main_Draw, "Draw function not defined.");

	while (cnMain_IsRunning() && !cnMain_IsTickLimitReached())
	{
		// Event checking should be quick.  Always processing events prevents
		// slowness due to bursts.
		cnUI_ProcessWindowEvents();

		uint64_t dt;
		if (cnMain_GenerateTick(&dt)) {
			Main_Tick(dt);
			cnMain_TickCompleted();
		}
		Main_Draw();
	}
}

void cnMain_Shutdown(void)
{
	if (Main_Shutdown) Main_Shutdown();

	cnR_Shutdown();
	cnUI_Shutdown();
	cnAssets_Shutdown();
	cnMem_Shutdown();
	cnLog_Shutdown();
}
