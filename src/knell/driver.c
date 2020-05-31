#include "driver.h"

#include <knell/assets.h>
#include <knell/assets-fileio.h>
#include <knell/control.h>
#include <knell/crash.h>
#include <knell/env.h>
#include <knell/input.h>
#include <knell/log.h>
#include <knell/memory.h>
#include <knell/plugin.h>
#ifdef _WIN32
#include <knell/process.h>
#endif
#include <knell/path.h>
#include <knell/tick-limits.h>
#include <knell/time.h>
#include <knell/render.h>
#include <knell/ui.h>

#include <errno.h>
#include <stdio.h>
#include <time.h>

static uint64_t lastTick;
static Plugin Payload;

int32_t parsePayload(int argc, char* argv[], int index, knDriverConfig* config);
int32_t parseAssetDir(int argc, char* argv[], int index, knDriverConfig* config);
int32_t parseTickLimit(int argc, char* argv[], int index, knDriverConfig* config);

CommandParser parsers[] = {
	{
		"-g,--game SHARED_LIB       Change the game/demo to boot.\n",
		"-g",
		"--game",
		parsePayload
	},
	{
		"-a,--asset-dir DIR         Change the directory for assets.\n",
		"-a",
		"--asset-dir",
		parseAssetDir
	},
	{
		"-t,--tick-limit NUM_TICKS  Limit the run to a specific number of ticks.\n",
		"-t",
		"--tick-limit",
		parseTickLimit
	}
};

int32_t parsePayload(int argc, char* argv[], int index, knDriverConfig* config)
{
	KN_ASSERT(config, "Cannot parse payload to a null knDriverConfig.");
	KN_ASSERT(0 < index && index < argc, "Argument index out of bounds: %d, num arguments: %d.", index, argc);
	if (index + 1 >= argc) {
		printf("Payload must be provided a shared library (or DLL) to load\n");
		return KN_ARG_PARSE_ERROR;
	}

	if (strlen(argv[index + 1]) < KN_PATH_MAX) {
		if (!Path_IsFile(argv[index + 1])) {
			char cwd[KN_PATH_MAX + 1];
			Env_CurrentWorkingDirectory(cwd, KN_PATH_MAX + 1);
			printf("Current working directory is: %s\n", cwd);
			printf("Game library %s does not exist.\n", argv[index + 1]);
			return KN_ARG_PARSE_ERROR;
		}
		PathBuffer_Set(&config->gameLibPath, argv[index + 1]);
		printf("Game library: '%s'\n", config->gameLibPath.str);
		return 2;
	}
	else {
		printf( "Length of name of game library is too long.");
		return KN_ARG_PARSE_ERROR;
	}
}

int32_t parseAssetDir(int argc, char* argv[], int index, knDriverConfig* config)
{
	KN_ASSERT(config, "Cannot parse asset dir to a null knDriverConfig.");
	KN_ASSERT(0 < index && index < argc, "Argument index out of bounds: %d, num arguments: %d.", index, argc);
	if (index + 1 >= argc) {
		printf("Must provide an asset directory to use.");
		return KN_ARG_PARSE_ERROR;
	}

	if (strlen(argv[index + 1]) < KN_PATH_MAX) {
		if (!Path_IsDir(argv[index + 1])) {
			printf("Asset directory %s does not exist\n", argv[index + 1]);
			return KN_ARG_PARSE_ERROR;
		}
		PathBuffer_Set(&config->assetDirPath, argv[index + 1]);
		printf("Asset path: '%s'\n", config->assetDirPath.str);
		return 2;
	}
	else {
		printf( "The asset path is too long.");
		return KN_ARG_PARSE_ERROR;
	}
}

int32_t parseTickLimit(int argc, char* argv[], int index, knDriverConfig* config)
{
	KN_ASSERT(config, "Cannot parse tick limit a null knDriverConfig.");
	KN_ASSERT(0 < index && index < argc, "Argument index out of bounds: %d, num arguments: %d.", index, argc);
	if (index + 1 >= argc) {
		printf("Must provide the number of ticks for which to run the program.");
		return KN_ARG_PARSE_ERROR;
	}
	char* readCursor;
	int64_t parsedValue = strtoll(argv[index + 1], &readCursor, 10);
	if (parsedValue < 0) {
		printf("Cannot step a negative number of ticks: %s\n", argv[index + 1]);
		return KN_ARG_PARSE_ERROR;
	}
	config->tickLimit = parsedValue;

	if (*readCursor != '\0' || errno == ERANGE) {
		printf("Unable to parse tick limit: %s\n", argv[index + 1]);
		return KN_ARG_PARSE_ERROR;
	}
	index += 2;
	return 2;
}

void Main_PrintUsage(void)
{
	printf("\nUsage: knell\n");
	for (uint32_t i = 0; i < KN_ARRAY_SIZE(parsers); ++i) {
		printf("%s", parsers[i].help);
	}
	printf("\n");
}

void Main_DescribeEnv(void)
{
	char buffer[1024];
	Env_CurrentWorkingDirectory(buffer, 1024);
	KN_TRACE(LogSysMain, "CWD: '%s'", buffer);
}

void Main_LoadPayload(const char* sharedLibraryName)
{
	KN_ASSERT(sharedLibraryName, "Cannot use a null shared library name to load a payload.");
	uint64_t gameLibModified;
	if (!Assets_LastModifiedTime(sharedLibraryName, &gameLibModified)) {
		KN_FATAL_ERROR("Unable to determine last modified time of '%s'", sharedLibraryName);
	}

	struct tm *lt = localtime((time_t*)&gameLibModified);
	char timeBuffer[80];
	strftime(timeBuffer, sizeof(timeBuffer), "%c", lt);
	KN_TRACE(LogSysMain, "Last modified time: %s", timeBuffer);

	if (Payload.shutdown) {
		Payload.shutdown();
	}

	SharedLibrary_Release(Payload.sharedLibrary);
	Payload.sharedLibrary = SharedLibrary_Load(sharedLibraryName);

	if (!Plugin_LoadFromFile(&Payload, sharedLibraryName)) {
		KN_FATAL_ERROR("Unable to load game module: %s", sharedLibraryName);
	}

	if (!Payload.init) KN_FATAL_ERROR("Plugin_Init function missing in %s", sharedLibraryName);
	if (!Payload.draw) KN_FATAL_ERROR("Plugin_DrawFn function missing in %s", sharedLibraryName);
	if (!Payload.tick) KN_FATAL_ERROR("Plugin_TickFn function missing in %s", sharedLibraryName);
	if (!Payload.shutdown) KN_FATAL_ERROR("Plugin_ShutdownFn function missing in %s", sharedLibraryName);

	if (!Payload.init()) {
		KN_FATAL_ERROR("%s failed to initialize", sharedLibraryName);
	}
}

/**
 * Common initialization point for all global systems.
 */
void Main_InitAllSystems(knDriverConfig* config)
{
	KN_ASSERT(config, "Cannot initialize using a null knDriverConfig.");
	Log_Init();
	Crash_Init();
	Mem_Init();
	Time_Init();

	if (strlen(config->assetDirPath.str) != 0) {
		Assets_Init(config->assetDirPath.str);
	}
	else {
		// If no asset directory was provided, look for `$KNELL_HOME/assets`, or
		// for an `assets/` directory in the working directory.
		PathBuffer defaultAssetDir;
		if (!Env_DefaultKnellHome(&defaultAssetDir)) {
			KN_FATAL_ERROR("Unable to determine the current Knell home directory.");
		}
		if (!PathBuffer_Join(&defaultAssetDir, "assets")) {
			KN_FATAL_ERROR("Unable to assemble a default asset directory name.");
		}
		Assets_Init(defaultAssetDir.str);
	}

	if (!Path_IsFile(config->gameLibPath.str)) {
		KN_FATAL_ERROR("Cannot load game. '%s' is not a game library.", config->gameLibPath.str);
	}

	const uint32_t width = 1024;
	const uint32_t height = 768;
	UI_Init(width, height);
	R_Init(width, height);

	const char* gameLib = config->gameLibPath.str;
	Main_LoadPayload(gameLib);
	Main_SetTickLimit(config->tickLimit);

	lastTick = Time_NowNs();

	KN_TRACE(LogSysMain, "Systems initialized.");

#ifdef _WIN32
	// TODO: This should be hidden unless doing an "diagnostic-startup-crash" or some other special behavior.
	//Proc_PrintLoadedDLLs();
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

/**
 * Parses a set of arguments into a driver configuration.
 */
bool knDriver_ParseCommandLine(int argc, char* argv[], knDriverConfig* config)
{
	KN_ASSERT(argc >= 1, "Argument count must at least include the executable.");
	KN_ASSERT(argv, "Cannot parser null arguments.");
	KN_ASSERT(config, "Cannot parse arguments to a null knDriverConfig.");

	PathBuffer_Clear(&config->gameLibPath);
	PathBuffer_Clear(&config->assetDirPath);

	// The log system is not initialized at this point, so using printf and
	// printf for now.
	int32_t argIndex = 1;
	while (argIndex < argc) {
		for (uint32_t parserIndex = 0; parserIndex < KN_ARRAY_SIZE(parsers); ++parserIndex) {
			if ((parsers[parserIndex].shortOption && strcmp(argv[argIndex], parsers[parserIndex].shortOption) == 0)
				|| (parsers[parserIndex].longOption && strcmp(argv[argIndex], parsers[parserIndex].longOption) == 0))
			{
				const int32_t argsParsed = parsers[parserIndex].parser(argc, argv, argIndex, config);
				if (argsParsed == KN_ARG_PARSE_ERROR) {
					Main_PrintUsage();
					return false;
				}
				argIndex += argsParsed;
			}
		}
		if (argIndex != argc) {
			printf("Unknown command line option\n");
			printf("Only parsed %d of %d arguments\n", argIndex, argc);
			Main_PrintUsage();
			return false;
		}
	}
	return true;
}

void knDriver_Init(knDriverConfig* config)
{
	KN_ASSERT(config, "Cannot initialize a driver with a null knDriverConfig.");
	Main_DescribeEnv();
	Main_InitAllSystems(config);
}

/**
 * The big loop which processes events, ticks and draws until the game is ready
 * to shut down.
 */
void knDriver_MainLoop(void)
{
	while (Main_IsRunning() && !Main_IsTickLimitReached())
	{
		// Event checking should be quick.  Always processing events prevents
		// slowness due to bursts.
		UI_ProcessWindowEvents();

		uint64_t dt;
		if (Main_GenerateTick(&dt)) {
			Payload.tick(dt);
			Main_TickCompleted();
		}
		Payload.draw();
	}
}

void knDriver_Shutdown(void)
{
	Payload.shutdown();
	R_Shutdown();
	UI_Shutdown();
	Assets_Shutdown();
	Mem_Shutdown();
	Log_Shutdown();
}
