#include "main.h"

#include <calendon/argparse.h>
#include <calendon/assets.h>
#include <calendon/assets-fileio.h>
#include <calendon/control.h>
#include <calendon/crash.h>
#include <calendon/env.h>
#include <calendon/log.h>
#include <calendon/main-config.h>
#include <calendon/memory.h>
#include <calendon/plugin.h>
#ifdef _WIN32
#include <calendon/process.h>
#endif
#include <calendon/path.h>
#include <calendon/string.h> // for config
#include <calendon/system.h>
#include <calendon/tick-limits.h>
#include <calendon/time.h>
#include <calendon/render.h>
#include <calendon/ui.h>

#include <errno.h>
#include <stdio.h>
#include <time.h>

static CnTime lastTick;
static CnPlugin Payload;

static CnPlugin_InitFn Main_Init;
static CnPlugin_TickFn Main_Tick;
static CnPlugin_DrawFn Main_Draw;
static CnPlugin_ShutdownFn Main_Shutdown;

enum { CnMaxNumCoreSystems = 16 };
static CnSystem coreSystems[CnMaxNumCoreSystems];
static uint32_t numCoreSystems = 0;

void cnMain_AddCoreSystem(CnSystem system)
{
	CN_ASSERT(numCoreSystems < CnMaxNumCoreSystems, "Too many core systems added.");
	coreSystems[numCoreSystems] = system;
	++numCoreSystems;
};

CnMainConfig s_config;

bool cnMain_Init(void) {
	if (s_config.tickLimit != 0) {
		cnMain_SetTickLimit(s_config.tickLimit);
	}
	return true;
}

CnPlugin cnMain_Plugin(void) {
	return (CnPlugin) {
		.init = cnMain_Init,
		.shutdown = NULL,
		.tick = NULL,
		.draw = NULL,
		.sharedLibrary = NULL
	};
}

int32_t cnMain_Payload(const CnCommandLineParse* parse, void* c)
{
	CN_ASSERT_NOT_NULL(parse);
	CN_ASSERT_NOT_NULL(c);

	CnMainConfig* config = (CnMainConfig*)c;

	if (!cnCommandLineParse_HasLookAhead(parse, 2)) {
		printf("Payload must be provided a shared library (or DLL) to load\n");
		return CnOptionParseError;
	}

	const char* gamePath = cnCommandLineParse_LookAhead(parse, 2);
	if (cnString_TerminatedFitsIn(gamePath, CN_MAX_TERMINATED_PATH)) {
		if (!cnPath_IsFile(gamePath)) {
			char cwd[CN_MAX_TERMINATED_PATH];
			cnEnv_CurrentWorkingDirectory(cwd, CN_MAX_TERMINATED_PATH);
			printf("Current working directory is: %s\n", cwd);
			printf("Game library %s does not exist.\n", gamePath);
			return CnOptionParseError;
		}
		cnPathBuffer_Set(&config->gameLibPath, gamePath);
		printf("Game library: '%s'\n", config->gameLibPath.str);
		return 2;
	}
	else {
		printf( "Length of name of game library is too long.");
		return CnOptionParseError;
	}
}

int32_t cnMain_OptionTickLimit(const CnCommandLineParse* parse, void* c)
{
	CN_ASSERT_NOT_NULL(parse);
	CN_ASSERT_NOT_NULL(c);

	CnMainConfig* config = (CnMainConfig*)c;

	if (!cnCommandLineParse_HasLookAhead(parse, 2)) {
		printf("Must provide the number of ticks for which to run the program.\n");
		return CnOptionParseError;
	}

	const char* tickString = cnCommandLineParse_LookAhead(parse, 2);
	char* readCursor;
	const int64_t parsedValue = strtoll(tickString, &readCursor, 10);
	if (parsedValue < 0) {
		printf("Cannot step a negative number of ticks: %s\n", tickString);
		return CnOptionParseError;
	}

	if (*readCursor != '\0' || errno == ERANGE) {
		printf("Unable to parse tick limit: %s\n", tickString);
		return CnOptionParseError;
	}
	config->tickLimit = parsedValue;
	return 2;
}

static CnCommandLineOption s_options[] = {
	{
		"-g,--game SHARED_LIB       Change the game/demo to boot.\n",
		"-g",
		"--game",
		cnMain_Payload
	},
	{
		"-t,--tick-limit NUM_TICKS  Limit the run to a specific number of ticks.\n",
		"-t",
		"--tick-limit",
		cnMain_OptionTickLimit
	}
};

void* cnMain_Config(void)
{
	return &s_config;
}

void cnMain_SetDefaultConfig(void* config)
{
	CnMainConfig* c = (CnMainConfig*)config;
	memset(c, 0, sizeof(CnMainConfig));
}

CnCommandLineOptionList cnMain_CommandLineOptionList(void)
{
	return (CnCommandLineOptionList) {
		.options = s_options,
		.numOptions = 2
	};
}

CnSystem cnMain_System(void)
{
	return (CnSystem) {
		.name = "Main",
		.options = cnMain_CommandLineOptionList,
		.config = cnMain_Config,
		.setDefaultConfig = cnMain_SetDefaultConfig,
		.plugin = cnMain_Plugin
	};
}

static void cnMain_BuildCoreSystemList(void)
{
	CnSystem_SystemFn systems[] = {
		cnMain_System,
		cnLog_System,
		cnCrash_System,
		cnMem_System,
		cnTime_System,
		cnAssets_System
	};

	for (uint32_t i = 0; i < CN_ARRAY_SIZE(systems); ++i) {
		cnMain_AddCoreSystem(systems[i]());
	}
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


bool cnMain_ParseCommandLine(int argc, char** argv)
{
	CN_ASSERT(argc >= 1, "Argument count must at least include the executable.");
	CN_ASSERT(argv, "Cannot parse null arguments.");

	for (uint32_t i = 0; i < numCoreSystems; ++i) {
		if (coreSystems[i].setDefaultConfig == NULL) {
			CN_ERROR(LogSysMain, "%s is missing a default config.", coreSystems[i].name);
		}
		CN_ASSERT_NOT_NULL(coreSystems[i].setDefaultConfig);
		coreSystems[i].setDefaultConfig(coreSystems[i].config());
	}

	CnCommandLineParse commandLineParse = cnCommandLineParse_Make(argc, argv);

	// The log system is not initialized at this point, so use printf.
	while (cnCommandLineParse_ShouldContinue(&commandLineParse)) {
		bool parseAdvanced = false;
		for (uint32_t systemIndex = 0; systemIndex < numCoreSystems; ++systemIndex) {
			CnSystem* system = &coreSystems[systemIndex];
			CnCommandLineOptionList options = system->options();
			for (uint32_t parserIndex = 0; parserIndex < options.numOptions; ++parserIndex) {
				CnCommandLineOption* option = &options.options[parserIndex];
				if (cnCommandLineOption_Matches(option, &commandLineParse)) {
					const int32_t argsParsed = option->parser(&commandLineParse, system->config());
					if (argsParsed == CnOptionParseError) {
						cnArgparse_PrintUsage(argc, argv);
						return false;
					}
					cnCommandLineParse_Advance(&commandLineParse, argsParsed);
					parseAdvanced = true;
					break;
				}
			}
			if (cnCommandLineParse_IsComplete(&commandLineParse)) {
				break;
			}
		}
		if (!parseAdvanced) {
			printf("Unable to parse argument: \"%s\" at index %d\n",
				cnCommandLineParse_LookAhead(&commandLineParse, 1),
				cnCommandLineParse_LookAheadIndex(&commandLineParse, 1));
			break;
		}
	}
	if (!cnCommandLineParse_IsComplete(&commandLineParse)) {
		printf("Unknown command line option\n");
		printf("Only parsed %d of %d arguments\n",
			cnCommandLineParse_LookAheadIndex(&commandLineParse, 1),
			argc);
		cnArgparse_PrintUsage(argc, argv);
		return false;
	}
	return true;
}

/**
 * Common initialization point for all global systems.
 */
void cnMain_InitAllSystems(int argc, char** argv)
{
	cnMain_BuildCoreSystemList();

	if (!cnMain_ParseCommandLine(argc, argv)) {
		CN_FATAL_ERROR("Unable to parse command line.");
	}

	for (uint32_t i = 0; i < numCoreSystems; ++i) {
		if (!coreSystems[i].plugin().init()) {
			CN_FATAL_ERROR("Unable to initialize core system: %d", i);
		}
	}

#if CN_USE_CONFIG_FILES
	cnConfig_Init();
	{
		// Read every configuration file.
		CnConfigFile file = cnConfigFile_Load(fileName);

		// Apply each attribute in turn.
		CnConfigKey key = cnConfig_NextKey();
		CnConfigLine line = cnConfig_NextLine();

		const char* prefix = cnConfigKey_Prefix(key);
		CnSystemId id = cnConfig_PrefixToSystemId(prefix);

		const char8 value = cnConfigKey_Value(key);

		CnSystem* system;
		system->Configure();
	}
#endif

	// TODO: Resolution should be read from config or as a a configuration option.
	const uint32_t width = 1024;
	const uint32_t height = 768;

	CnUIInitParams uiInitParams;
	uiInitParams.resolution = (CnDimension2u32) { .width = width, .height = height };

	cnUI_Init(&uiInitParams);
	cnR_Init(uiInitParams.resolution);

	// If there is a demo to load from file, then use that.
	if (!cnPlugin_IsComplete(&s_config.payload)) {
		if (!cnPath_IsFile(s_config.gameLibPath.str)) {
			CN_FATAL_ERROR("Cannot load game. '%s' is not a game library.", s_config.gameLibPath.str);
		}

		const char* gameLib = s_config.gameLibPath.str;
		if (gameLib) {
			cnMain_LoadPayloadFromFile(gameLib);
		}
	}
	else {
		cnMain_RegisterPayload(&s_config.payload);
	}

	if (Main_Init) {
		Main_Init();
	}
	if (!Main_Draw) CN_FATAL_ERROR("Draw function missing. Write a Main_Draw(void) function.");
	if (!Main_Tick) CN_FATAL_ERROR("Update function missing. Write a Main_Tick(void) function.");

	//cnMain_SetTickLimit(s_config.tickLimit);

	lastTick = cnTime_MakeNow();

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
bool cnMain_GenerateTick(CnTime* outDt)
{
	const CnTime current = cnTime_Max(lastTick, cnTime_MakeNow());

	// Prevent updating too rapidly.  Maintaining a relatively consistent
	// timestep limits stored state and prevents precision errors due to
	// extremely small dt.
	//
	// Since Calendon is single-threaded, VSync will probably ensure that the
	// minimum tick size is never missed.
	const CnTime minTickSize = cnTime_MakeMilli(8);
	const CnTime dt = cnTime_SubtractMonotonic(current, lastTick);
	if (cnTime_LessThan(dt, minTickSize)) {
		return false;
	}

	lastTick = current;

	// Ignore huge ticks, such as when resuming in the debugger.
	const CnTime maxTickSize = cnTime_MakeMilli(5000);
	if (cnTime_LessThan(maxTickSize, dt)) {
		CN_TRACE(LogSysMain, "Skipping large tick: %" PRIu64 " ms", cnTime_Milli(dt));
		return false;
	}

	*outDt = dt;
	return true;
}

void cnMain_StartUp(int argc, char** argv)
{
	cnMain_DescribeEnv();
	cnMain_InitAllSystems(argc, argv);
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

		CnTime dt;
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

	for (uint32_t i = 0; i < numCoreSystems; ++i) {
		const uint32_t nextSystemIndex = numCoreSystems - i - 1;

		CnSystem* system = &coreSystems[nextSystemIndex];
		printf("Shutting down: %s\n", system->name);
		if (!system->plugin().shutdown) {
			printf("No shutdown function for: %s\n", system->name);
		}
		else {
			system->plugin().shutdown();
		}
	}
}
