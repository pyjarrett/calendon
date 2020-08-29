#include "main-config.h"

#include <calendon/cn.h>
#include <calendon/path.h>
#include <calendon/string.h>

#include <errno.h>
#include <string.h>

int32_t cnMain_OptionPrintWorkingDirectory(const CnCommandLineParse* parse, void* c);
int32_t cnMain_OptionPayload(const CnCommandLineParse* parse, void* c);
int32_t cnMain_OptionTickLimit(const CnCommandLineParse* parse, void* c);
int32_t cnMain_OptionHeadless(const CnCommandLineParse* parse, void* c);

static CnMainConfig s_config;
static CnCommandLineOption s_options[] = {
	{
		"\t--pwd\n"
		"\t\tPrint the current working directory.\n",
		NULL,
		"--pwd",
		cnMain_OptionPrintWorkingDirectory
	},
	{
		"\t-g,--game SHARED_LIB\n"
			"\t\tChange the game/demo to boot.\n",
		"-g",
		"--game",
		cnMain_OptionPayload
	},
	{
		"\t-t,--tick-limit NUM_TICKS\n"
			"\t\tLimit the run to a specific number of ticks.\n",
		"-t",
		"--tick-limit",
		cnMain_OptionTickLimit
	},
	{
		"\t--headless\n"
		"\t\tRun without starting up the UI or the renderer.\n",
		NULL,
		"--headless",
		cnMain_OptionHeadless
	}
};

CnCommandLineOptionList cnMain_CommandLineOptionList(void)
{
	return (CnCommandLineOptionList) {
		.options = s_options,
		.numOptions = 4
	};
}

void* cnMain_Config(void)
{
	return &s_config;
}

void cnMain_SetDefaultConfig(void* config)
{
	CnMainConfig* c = (CnMainConfig*)config;
	memset(c, 0, sizeof(CnMainConfig));
	c->headless = false;
}

int32_t cnMain_OptionPrintWorkingDirectory(const CnCommandLineParse* parse, void* config)
{
	CN_ASSERT_PTR(parse);
	CN_ASSERT_PTR(config);

	CN_UNUSED(parse);
	CN_UNUSED(config);

	CnPathBuffer cwd;
	if (!cnPathBuffer_CurrentWorkingDirectory(&cwd)) {
		CN_FATAL_ERROR("Unable to get current working directory.");
	}
	cnPrint("Current working directory: '%s'\n", cwd);

	return 1;
}

int32_t cnMain_OptionPayload(const CnCommandLineParse* parse, void* c)
{
	CN_ASSERT_PTR(parse);
	CN_ASSERT_PTR(c);

	CnMainConfig* config = (CnMainConfig*)c;

	if (!cnCommandLineParse_HasLookAhead(parse, 2)) {
		cnPrint("Payload must be provided a shared library (or DLL) to load\n");
		return CnOptionParseError;
	}

	const char* gamePath = cnCommandLineParse_LookAhead(parse, 2);
	if (cnString_FitsWithNull(gamePath, CN_MAX_TERMINATED_PATH)) {
		if (!cnPath_IsFile(gamePath)) {
			CnPathBuffer cwd;
			if (!cnPathBuffer_CurrentWorkingDirectory(&cwd)) {
				CN_FATAL_ERROR("Unable to get current working directory.");
			}
			cnPrint("Current working directory is: %s\n", cwd.str);
			cnPrint("Game library %s does not exist.\n", gamePath);
			return CnOptionParseError;
		}
		cnPathBuffer_Set(&config->gameLibPath, gamePath);
		cnPrint("Game library: '%s'\n", config->gameLibPath.str);
		return 2;
	}
	else {
		cnPrint( "Length of name of game library is too long.");
		return CnOptionParseError;
	}
}

int32_t cnMain_OptionTickLimit(const CnCommandLineParse* parse, void* c)
{
	CN_ASSERT_PTR(parse);
	CN_ASSERT_PTR(c);

	CnMainConfig* config = (CnMainConfig*)c;

	if (!cnCommandLineParse_HasLookAhead(parse, 2)) {
		cnPrint("Must provide the number of ticks for which to run the program.\n");
		return CnOptionParseError;
	}

	const char* tickString = cnCommandLineParse_LookAhead(parse, 2);
	char* readCursor;
	const int64_t parsedValue = strtoll(tickString, &readCursor, 10);
	if (parsedValue < 0) {
		cnPrint("Cannot step a negative number of ticks: %s\n", tickString);
		return CnOptionParseError;
	}

	if (*readCursor != '\0' || errno == ERANGE) {
		cnPrint("Unable to parse tick limit: %s\n", tickString);
		return CnOptionParseError;
	}
	config->tickLimit = parsedValue;
	return 2;
}

int32_t cnMain_OptionHeadless(const CnCommandLineParse* parse, void* c)
{
	CN_ASSERT_PTR(parse);
	CN_ASSERT_PTR(c);

	CnMainConfig* config = (CnMainConfig*)c;
	config->headless = true;

	return 1;
}

void cnMainConfig_Freestanding(CnPlugin_InitFn init, CnPlugin_TickFn tick,
	CnPlugin_DrawFn draw, CnPlugin_ShutdownFn shutdown)
{
	s_config.payload.init = init;
	s_config.payload.tick = tick;
	s_config.payload.draw = draw;
	s_config.payload.shutdown = shutdown;
	s_config.payload.sharedLibrary = NULL;
}
