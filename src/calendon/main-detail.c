#include "main-detail.h"

#include <calendon/assets.h>
#include <calendon/assets-fileio.h>
#include <calendon/crash.h>
#include <calendon/log.h>
#include <calendon/main-config.h>
#include <calendon/memory.h>
#include <calendon/plugin.h>
#ifdef _WIN32
#include <calendon/process.h>
#endif
#include <calendon/system.h>
#include <calendon/tick-limits.h>
#include <calendon/time.h>
#include <calendon/render.h>
#include <calendon/ui.h>

#include <time.h>

CnTime s_lastTick;
CnPlugin s_payload;
CnSystem s_coreSystems[CnMaxNumCoreSystems];
uint32_t s_numCoreSystems = 0;

static bool cnMain_Init(void) {
	CnMainConfig* config = (CnMainConfig*)cnMain_Config();
	if (config->tickLimit != 0) {
		cnMain_SetTickLimit(config->tickLimit);
	}
	return true;
}

static CnPlugin cnMain_Plugin(void) {
	return (CnPlugin) {
		.init          = cnMain_Init,
		.shutdown      = NULL,
		.tick          = NULL,
		.draw          = NULL,
		.sharedLibrary = NULL
	};
}

static CnSystem cnMain_System(void)
{
	return (CnSystem) {
		.name             = "Main",
		.options          = cnMain_CommandLineOptionList,
		.config           = cnMain_Config,
		.setDefaultConfig = cnMain_SetDefaultConfig,
		.plugin           = cnMain_Plugin
	};
}

void cnMain_AddCoreSystem(CnSystem system)
{
	CN_ASSERT(s_numCoreSystems < CnMaxNumCoreSystems, "Too many core systems added.");
	s_coreSystems[s_numCoreSystems] = system;
	++s_numCoreSystems;
};

void cnMain_BuildCoreSystemList(void)
{
	CnSystem_SystemFn systems[] = {
		cnMain_System,
		cnLog_System,
		cnCrash_System,
		cnMemory_System,
		cnTime_System,
		cnAssets_System
	};

	for (uint32_t i = 0; i < CN_ARRAY_SIZE(systems); ++i) {
		cnMain_AddCoreSystem(systems[i]());
	}
}

void cnMain_ValidatePayload(CnPlugin* payload)
{
	CN_ASSERT(payload, "Cannot register a null payload.");

	if (!payload->init) CN_FATAL_ERROR("CnPlugin_Init function missing in payload.");
	if (!payload->draw) CN_FATAL_ERROR("CnPlugin_DrawFn function missing in payload.");
	if (!payload->tick) CN_FATAL_ERROR("CnPlugin_TickFn function missing in payload.");
	if (!payload->shutdown) CN_FATAL_ERROR("CnPlugin_ShutdownFn function missing in payload.");
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

	// Shutdown any previous plugin.
	if (s_payload.shutdown) {
		s_payload.shutdown();
	}

	cnSharedLibrary_Release(s_payload.sharedLibrary);
	CnSharedLibrary library = cnSharedLibrary_Load(sharedLibraryName);
	if (!library) {
		CN_FATAL_ERROR("Unable to load game module: %s", sharedLibraryName);
	}
	cnPlugin_LoadFromSharedLibrary(&s_payload, library);

	cnMain_ValidatePayload(&s_payload);
}

void cnMain_PrintUsage(int argc, char** argv)
{
	// Print core systems.
	for (uint32_t i = 0; i < s_numCoreSystems; ++i) {
		CnSystem* system = &s_coreSystems[i];
		CnCommandLineOptionList optionList = system->options();
		if (system->name && optionList.numOptions > 0) {
			cnPrint("%s\n", system->name);
		}

		for (uint32_t optionIndex = 0; optionIndex < optionList.numOptions; ++optionIndex) {
			CnCommandLineOption* option = &optionList.options[optionIndex];
			const char* help = option->help ? option->help : "";
			cnPrint("%s\n", help);
		}
	}

	// Print the provided arguments.
	cnPrint("Arguments provided:\n");
	for (int i = 0; i < argc; ++i) {
		cnPrint("%4d: \"%s\"\n", i, argv[i]);
	}
}

/**
 * Attempt to parse the next command line option with a given system.
 */
int32_t cnMain_RunSystemParsers(CnCommandLineParse* commandLineParse, CnSystem* system)
{
	CN_ASSERT_PTR(commandLineParse);
	CN_ASSERT_PTR(system);

	CnCommandLineOptionList options = system->options();
	for (uint32_t parserIndex = 0; parserIndex < options.numOptions; ++parserIndex) {
		CnCommandLineOption *option = &options.options[parserIndex];
		if (cnCommandLineOption_Matches(option, commandLineParse)) {
			return option->parser(commandLineParse, system->config());
		}
	}
	return 0;
}

/**
 * Parses the raw input, as such received by `int main(int argc, char** argv)`.
 */
bool cnMain_ParseCommandLine(int argc, char** argv)
{
	CN_ASSERT(argc >= 1, "Argument count must at least include the executable.");
	CN_ASSERT_PTR(argv);

	// Default the configs for systems before they begin receiving changes from
	// the command line.
	for (uint32_t i = 0; i < s_numCoreSystems; ++i) {
		if (s_coreSystems[i].setDefaultConfig == NULL) {
			CN_ERROR(LogSysMain, "%s is missing a default config.", s_coreSystems[i].name);
		}
		CN_ASSERT_PTR(s_coreSystems[i].setDefaultConfig);
		s_coreSystems[i].setDefaultConfig(s_coreSystems[i].config());
	}

	CnCommandLineParse commandLineParse = cnCommandLineParse_Make(argc, argv);
	while (cnCommandLineParse_ShouldContinue(&commandLineParse)) {
		int argsParsed = 0;
		uint32_t systemIndex = 0;
		while (systemIndex < s_numCoreSystems && argsParsed == 0) {
			CnSystem* system = &s_coreSystems[systemIndex];
			argsParsed = cnMain_RunSystemParsers(&commandLineParse, system);
			if (argsParsed != 0) {
				break;
			}
			++systemIndex;
		}
		if (argsParsed <= 0) {
			cnPrint("Unable to parse argument: \"%s\" at index %d\n",
				cnCommandLineParse_LookAhead(&commandLineParse, 1),
				cnCommandLineParse_LookAheadIndex(&commandLineParse, 1));
			cnMain_PrintUsage(argc, argv);
			return false;
		}
		cnCommandLineParse_Advance(&commandLineParse, argsParsed);
	}
	return true;
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
	const CnTime current = cnTime_Max(s_lastTick, cnTime_MakeNow());

	// Prevent updating too rapidly.  Maintaining a relatively consistent
	// timestep limits stored state and prevents precision errors due to
	// extremely small dt.
	//
	// Since Calendon is single-threaded, VSync will probably ensure that the
	// minimum tick size is never missed.
	const CnTime minTickSize = cnTime_MakeMilli(8);
	const CnTime dt = cnTime_SubtractMonotonic(current, s_lastTick);
	if (cnTime_LessThan(dt, minTickSize)) {
		return false;
	}

	s_lastTick = current;

	// Ignore huge ticks, such as when resuming in the debugger.
	const CnTime maxTickSize = cnTime_MakeMilli(5000);
	if (cnTime_LessThan(maxTickSize, dt)) {
		CN_TRACE(LogSysMain, "Skipping large tick: %" PRIu64 " ms", cnTime_Milli(dt));
		return false;
	}

	*outDt = dt;
	return true;
}

void cnMain_StartUpUI(void)
{
	// TODO: Resolution should be read from config or as a a configuration option.
	const uint32_t width = 1024;
	const uint32_t height = 768;

	CnUIInitParams uiInitParams;
	uiInitParams.resolution = (CnDimension2u32) { .width = width, .height = height };

	cnUI_Init(&uiInitParams);
	cnR_Init(uiInitParams.resolution);
}
