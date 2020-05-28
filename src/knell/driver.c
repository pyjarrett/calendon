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

typedef struct {
	PathBuffer gameLibPath;
	PathBuffer assetDirPath;
} MainConfig;

static MainConfig mainConfig;
static Plugin Payload;

void Main_PrintUsage(void)
{
	printf("\nUsage: knell\n");
	printf("  -a,--asset-dir DIR         Change the directory for assets.\n");
	printf("  -g,--game SHARED_LIB       Change the game/demo to boot.\n");
	printf("  -t,--tick-limit NUM_TICKS  Limit the run to a specific number of ticks.\n");
	printf("\n");
}

void Main_ParseCommandLineArguments(int argc, char* argv[])
{
	PathBuffer_Clear(&mainConfig.gameLibPath);
	PathBuffer_Clear(&mainConfig.assetDirPath);

	// The log system is not initialized at this point, so using printf and
	// printf for now.
	int32_t i = 1;
	while (i < argc) {
		// Game library name parameter.
		if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--game") == 0) {
			if (i + 1 >= argc) {
				printf("-g must be provided a shared library (or DLL) to load");
				exit(EXIT_FAILURE);
			}
			else {
				if (strlen(argv[i+1]) < KN_PATH_MAX) {
					if (!Path_IsFile(argv[i+1])) {
						char cwd[4096];
						Env_CurrentWorkingDirectory(cwd, 4096);
						printf("Current working directory is: %s\n", cwd);
						printf("Game library %s does not exist\n", argv[i+1]);
						exit(EXIT_FAILURE);
					}
					PathBuffer_Set(&mainConfig.gameLibPath, argv[i + 1]);
					printf("Game library: '%s'\n", mainConfig.gameLibPath.str);
					i += 2;
				}
				else {
					printf( "Length of name of game library is too long");
					exit(EXIT_FAILURE);
				}
			}
		}
			// Asset path name parameter
		else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--asset-dir") == 0) {
			if (i + 1 >= argc) {
				printf("-a provide an asset directory to use");
				exit(EXIT_FAILURE);
			}
			else {
				if (strlen(argv[i+1]) < KN_PATH_MAX) {
					if (!Path_IsDir(argv[i+1])) {
						printf("Asset directory %s does not exist\n", argv[i+1]);
						exit(EXIT_FAILURE);
					}

					PathBuffer_Set(&mainConfig.assetDirPath, argv[i + 1]);
					printf("Asset path: '%s'\n", mainConfig.assetDirPath.str);
					i += 2;
				}
				else {
					printf( "The asset path is too long.");
					exit(EXIT_FAILURE);
				}
			}
		}
		else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--tick-limit") == 0) {
			if (i + 1 >= argc) {
				printf("-t must provide the number of ticks for which to run the program");
				exit(EXIT_FAILURE);
			}
			char* readCursor;
			int64_t parsedValue = strtoll(argv[i + 1], &readCursor, 10);
			if (parsedValue < 0) {
				printf("Cannot step a negative number of ticks: %s\n", argv[i+1]);
				exit(EXIT_FAILURE);
			}
			Main_SetTickLimit(parsedValue);

			if (*readCursor != '\0' || errno == ERANGE) {
				printf("Unable to parse tick limit: %s\n", argv[i+1]);
				exit(EXIT_FAILURE);
			}
			i += 2;
		}
		else {
			printf("Unknown command line option\n");
			Main_PrintUsage();
			exit(EXIT_FAILURE);
		}
	}
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
void Main_InitAllSystems(void)
{
	Log_Init();
	Crash_Init();
	Mem_Init();
	Time_Init();

	if (strlen(mainConfig.assetDirPath.str) != 0) {
		Assets_Init(mainConfig.assetDirPath.str);
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

	if (!Path_IsFile(mainConfig.gameLibPath.str)) {
		KN_FATAL_ERROR("Cannot load game. '%s' is not a game library.", mainConfig.gameLibPath.str);
	}

	const uint32_t width = 1024;
	const uint32_t height = 768;
	UI_Init(width, height);
	R_Init(width, height);

	const char* gameLib = mainConfig.gameLibPath.str;
	Main_LoadPayload(gameLib);

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


void knDriver_Init(int argc, char** argv)
{
	Main_ParseCommandLineArguments(argc, argv);
	Main_DescribeEnv();
	Main_InitAllSystems();
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
