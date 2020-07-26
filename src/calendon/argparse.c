#include "argparse.h"

#include <calendon/cn.h>
#include <calendon/env.h>
#include <calendon/string.h>

#include <errno.h>
#include <string.h>

int32_t cnArgparse_Payload(const CnCommandLineParse* parse, CnMainConfig* config);
int32_t cnArgparse_AssetDir(const CnCommandLineParse* parse, CnMainConfig* config);
int32_t cnArgparse_TickLimit(const CnCommandLineParse* parse, CnMainConfig* config);

CnCommandLineOption parsers[] = {
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

int32_t cnArgparse_Payload(const CnCommandLineParse* parse, CnMainConfig* config)
{
	CN_ASSERT_NOT_NULL(parse);
	CN_ASSERT_NOT_NULL(config);

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

int32_t cnArgparse_AssetDir(const CnCommandLineParse* parse, CnMainConfig* config)
{
	CN_ASSERT_NOT_NULL(parse);
	CN_ASSERT_NOT_NULL(config);

	if (!cnCommandLineParse_HasLookAhead(parse, 2)) {
		printf("Must provide an asset directory to use.\n");
		return CnOptionParseError;
	}

	const char* assetDir = cnCommandLineParse_LookAhead(parse, 2);
	if (cnString_TerminatedFitsIn(assetDir, CN_MAX_TERMINATED_PATH)) {
		if (!cnPath_IsDir(assetDir)) {
			printf("Asset directory %s does not exist\n", assetDir);
			return CnOptionParseError;
		}
		cnPathBuffer_Set(&config->assetDirPath, assetDir);
		printf("Asset path: '%s'\n", config->assetDirPath.str);
		return 2;
	}
	else {
		printf( "The asset path is too long.");
		return CnOptionParseError;
	}
}

int32_t cnArgparse_TickLimit(const CnCommandLineParse* parse, CnMainConfig* config)
{
	CN_ASSERT_NOT_NULL(parse);
	CN_ASSERT_NOT_NULL(config);

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

/**
 * Return if the next argument to parse matches the command line option.
 */
bool cnCommandLineOption_Matches(const CnCommandLineOption* option, const CnCommandLineParse* parse)
{
	CN_ASSERT_NOT_NULL(option);
	CN_ASSERT_NOT_NULL(parse->argv);
	CN_ASSERT(cnCommandLineParse_ShouldContinue(parse), "No more options to match.");
	const char* nextArg = cnCommandLineParse_LookAhead(parse, 1);
	return (option->shortOption && (strcmp(nextArg, option->shortOption) == 0)
		|| (option->longOption && strcmp(nextArg, option->longOption) == 0));
}

void cnArgparse_PrintUsage(int argc, char** argv)
{
	printf("Provided:\n");
	for (int i = 1; i < argc; ++i) {
		printf("%3d: \"%s\"\n", i, argv[i]);
	}

	printf("\nUsage: calendon\n");
	for (uint32_t i = 0; i < CN_ARRAY_SIZE(parsers); ++i) {
		printf("%s", parsers[i].help);
	}
	printf("\n");
}
