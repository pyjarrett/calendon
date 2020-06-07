#include "driver.h"

#include <calendon/assets.h>
#include <calendon/assets-fileio.h>
#include <calendon/control.h>
#include <calendon/crash.h>
#include <calendon/env.h>
#include <calendon/input.h>
#include <calendon/log.h>
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

static CnPlugin_InitFn cnMain_Init;
static CnPlugin_TickFn cnMain_Tick;
static CnPlugin_DrawFn cnMain_Draw;
static CnPlugin_ShutdownFn cnMain_Shutdown;

int32_t cnArgparse_Payload(int argc, char** argv, int index, CnDriverConfig* config);
int32_t cnArgparse_AssetDir(int argc, char** argv, int index, CnDriverConfig* config);
int32_t cnArgparse_TickLimit(int argc, char** argv, int index, CnDriverConfig* config);

CnCommandParser parsers[] = {
	{
		"-g,--game SHARED_LIB       Change the game/demo to boot.\n",
		"-g",
		"--game",
		cnArgparse_Payload
	},
	{
		"-a,--asset-dir DIR         Change the directory for assets.\n",
		"-a",
		"--asset-dir",
		cnArgparse_AssetDir
	},
	{
		"-t,--tick-limit NUM_TICKS  Limit the run to a specific number of ticks.\n",
		"-t",
		"--tick-limit",
		cnArgparse_TickLimit
	}
};

int32_t cnArgparse_Payload(int argc, char** argv, int index, CnDriverConfig* config)
{
	CN_ASSERT(config, "Cannot parse payload to a null CnDriverConfig.");
	CN_ASSERT(0 < index && index < argc, "Argument index out of bounds: %d, num arguments: %d.", index, argc);
	if (index + 1 >= argc) {
		printf("Payload must be provided a shared library (or DLL) to load\n");
		return CN_ARG_PARSE_ERROR;
	}

	if (strlen(argv[index + 1]) < CN_PATH_MAX) {
		if (!cnPath_IsFile(argv[index + 1])) {
			char cwd[CN_PATH_MAX + 1];
			cnEnv_CurrentWorkingDirectory(cwd, CN_PATH_MAX + 1);
			printf("Current working directory is: %s\n", cwd);
			printf("Game library %s does not exist.\n", argv[index + 1]);
			return CN_ARG_PARSE_ERROR;
		}
		cnPathBuffer_Set(&config->gameLibPath, argv[index + 1]);
		printf("Game library: '%s'\n", config->gameLibPath.str);
		return 2;
	}
	else {
		printf( "Length of name of game library is too long.");
		return CN_ARG_PARSE_ERROR;
	}
}

int32_t cnArgparse_AssetDir(int argc, char** argv, int index, CnDriverConfig* config)
{
	CN_ASSERT(config, "Cannot parse asset dir to a null CnDriverConfig.");
	CN_ASSERT(0 < index && index < argc, "Argument index out of bounds: %d, num arguments: %d.", index, argc);
	if (index + 1 >= argc) {
		printf("Must provide an asset directory to use.");
		return CN_ARG_PARSE_ERROR;
	}

	if (strlen(argv[index + 1]) < CN_PATH_MAX) {
		if (!cnPath_IsDir(argv[index + 1])) {
			printf("Asset directory %s does not exist\n", argv[index + 1]);
			return CN_ARG_PARSE_ERROR;
		}
		cnPathBuffer_Set(&config->assetDirPath, argv[index + 1]);
		printf("Asset path: '%s'\n", config->assetDirPath.str);
		return 2;
	}
	else {
		printf( "The asset path is too long.");
		return CN_ARG_PARSE_ERROR;
	}
}

int32_t cnArgparse_TickLimit(int argc, char** argv, int index, CnDriverConfig* config)
{
	CN_ASSERT(config, "Cannot parse tick limit a null CnDriverConfig.");
	CN_ASSERT(0 < index && index < argc, "Argument index out of bounds: %d, num arguments: %d.", index, argc);
	if (index + 1 >= argc) {
		printf("Must provide the number of ticks for which to run the program.");
		return CN_ARG_PARSE_ERROR;
	}
	char* readCursor;
	int64_t parsedValue = strtoll(argv[index + 1], &readCursor, 10);
	if (parsedValue < 0) {
		printf("Cannot step a negative number of ticks: %s\n", argv[index + 1]);
		return CN_ARG_PARSE_ERROR;
	}

	if (*readCursor != '\0' || errno == ERANGE) {
		printf("Unable to parse tick limit: %s\n", argv[index + 1]);
		return CN_ARG_PARSE_ERROR;
	}
	config->tickLimit = parsedValue;
	return 2;
}

bool cnDriverConfig_IsHosted(CnDriverConfig* config)
{
	CN_ASSERT(config, "Cannot determine if null config is hosted.");
	return config->payload.init
		&& config->payload.tick
		&& config->payload.draw
		&& config->payload.shutdown;
}

void cnMain_PrintUsage(void)
{
	printf("\nUsage: calendon\n");
	for (uint32_t i = 0; i < CN_ARRAY_SIZE(parsers); ++i) {
		printf("%s", parsers[i].help);
	}
	printf("\n");
}

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

	cnMain_Init = payload->init;
	cnMain_Tick = payload->tick;
	cnMain_Draw = payload->draw;
	cnMain_Shutdown = payload->shutdown;
}

void cnMain_LoadPayload(const char* sharedLibraryName)
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

	if (cnMain_Shutdown) {
		cnMain_Shutdown();
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
void cnMain_InitAllSystems(CnDriverConfig* config)
{
	CN_ASSERT(config, "Cannot initialize using a null CnDriverConfig.");
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

	if (!cnDriverConfig_IsHosted(config)) {
		if (!cnPath_IsFile(config->gameLibPath.str)) {
			CN_FATAL_ERROR("Cannot load game. '%s' is not a game library.", config->gameLibPath.str);
		}

		const char* gameLib = config->gameLibPath.str;
		if (gameLib) {
			cnMain_LoadPayload(gameLib);
		}
	}

	if (cnMain_Init) {
		cnMain_Init();
	}
	if (!cnMain_Draw) CN_FATAL_ERROR("Draw function missing. Write a cnMain_Draw(void) function.");
	if (!cnMain_Tick) CN_FATAL_ERROR("Update function missing. Write a cnMain_Tick(void) function.");

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

/**
 * Parses a set of arguments into a driver configuration.
 */
bool cnDriverConfig_ParseCommandLine(CnDriverConfig* config, int argc, char* argv[])
{
	CN_ASSERT(argc >= 1, "Argument count must at least include the executable.");
	CN_ASSERT(argv, "Cannot parse null arguments.");
	CN_ASSERT(config, "Cannot parse arguments to a null CnDriverConfig.");

	cnPathBuffer_Clear(&config->gameLibPath);
	cnPathBuffer_Clear(&config->assetDirPath);

	memset(&config->payload, 0, sizeof(config->payload));
	config->tickLimit = 0;

	// The log system is not initialized at this point, so using printf and
	// printf for now.
	int32_t argIndex = 1;
	while (argIndex < argc) {
		for (uint32_t parserIndex = 0; parserIndex < CN_ARRAY_SIZE(parsers); ++parserIndex) {
			if ((parsers[parserIndex].shortOption && strcmp(argv[argIndex], parsers[parserIndex].shortOption) == 0)
				|| (parsers[parserIndex].longOption && strcmp(argv[argIndex], parsers[parserIndex].longOption) == 0))
			{
				const int32_t argsParsed = parsers[parserIndex].parser(argc, argv, argIndex, config);
				if (argsParsed == CN_ARG_PARSE_ERROR) {
					cnMain_PrintUsage();
					return false;
				}
				argIndex += argsParsed;
				break;
			}
		}
	}
	if (argIndex != argc) {
		printf("Unknown command line option\n");
		printf("Only parsed %d of %d arguments\n", argIndex, argc);
		cnMain_PrintUsage();
		return false;
	}
	return true;
}

void cnDriverConfig_Freestanding(CnDriverConfig* config, CnPlugin_InitFn init,
	CnPlugin_TickFn tick, CnPlugin_DrawFn draw, CnPlugin_ShutdownFn shutdown)
{
	CN_ASSERT(config, "Cannot create a freestanding implementation from a null config.");

	config->payload.init = init;
	config->payload.tick = tick;
	config->payload.draw = draw;
	config->payload.shutdown = shutdown;
	config->payload.sharedLibrary = NULL;

	if (!cnDriverConfig_IsHosted(config)) {
		CN_FATAL_ERROR("Improperly hosted config.");
	}

	cnMain_RegisterPayload(&config->payload);
}

void cnDriver_Init(CnDriverConfig* config)
{
	CN_ASSERT(config, "Cannot initialize a driver with a null CnDriverConfig.");
	cnMain_DescribeEnv();
	cnMain_InitAllSystems(config);
}

/**
 * The big loop which processes events, ticks and draws until the game is ready
 * to shut down.
 */
void cnDriver_MainLoop(void)
{
	CN_ASSERT(cnMain_Tick, "Tick function not defined.");
	CN_ASSERT(cnMain_Draw, "Draw function not defined.");

	while (cnMain_IsRunning() && !cnMain_IsTickLimitReached())
	{
		// Event checking should be quick.  Always processing events prevents
		// slowness due to bursts.
		cnUI_ProcessWindowEvents();

		uint64_t dt;
		if (cnMain_GenerateTick(&dt)) {
			cnMain_Tick(dt);
			cnMain_TickCompleted();
		}
		cnMain_Draw();
	}
}

void cnDriver_Shutdown(void)
{
	if (cnMain_Shutdown) cnMain_Shutdown();

	cnR_Shutdown();
	cnUI_Shutdown();
	cnAssets_Shutdown();
	cnMem_Shutdown();
	cnLog_Shutdown();
}
