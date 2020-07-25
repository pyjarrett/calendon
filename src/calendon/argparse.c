#include "argparse.h"

#include <calendon/cn.h>
#include <calendon/env.h>

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
		return CN_ARG_PARSE_ERROR;
	}

	const char* gamePath = cnCommandLineParse_LookAhead(parse, 2);
	if (strlen(gamePath) < CN_PATH_MAX) {
		if (!cnPath_IsFile(gamePath)) {
			char cwd[CN_PATH_MAX + 1];
			cnEnv_CurrentWorkingDirectory(cwd, CN_PATH_MAX + 1);
			printf("Current working directory is: %s\n", cwd);
			printf("Game library %s does not exist.\n", gamePath);
			return CN_ARG_PARSE_ERROR;
		}
		cnPathBuffer_Set(&config->gameLibPath, gamePath);
		printf("Game library: '%s'\n", config->gameLibPath.str);
		return 2;
	}
	else {
		printf( "Length of name of game library is too long.");
		return CN_ARG_PARSE_ERROR;
	}
}

int32_t cnArgparse_AssetDir(const CnCommandLineParse* parse, CnMainConfig* config)
{
	CN_ASSERT_NOT_NULL(parse);
	CN_ASSERT_NOT_NULL(config);

	if (!cnCommandLineParse_HasLookAhead(parse, 2)) {
		printf("Must provide an asset directory to use.\n");
		return CN_ARG_PARSE_ERROR;
	}

	const char* assetDir = cnCommandLineParse_LookAhead(parse, 2);
	if (strlen(assetDir) < CN_PATH_MAX) {
		if (!cnPath_IsDir(assetDir)) {
			printf("Asset directory %s does not exist\n", assetDir);
			return CN_ARG_PARSE_ERROR;
		}
		cnPathBuffer_Set(&config->assetDirPath, assetDir);
		printf("Asset path: '%s'\n", config->assetDirPath.str);
		return 2;
	}
	else {
		printf( "The asset path is too long.");
		return CN_ARG_PARSE_ERROR;
	}
}

int32_t cnArgparse_TickLimit(const CnCommandLineParse* parse, CnMainConfig* config)
{
	CN_ASSERT_NOT_NULL(parse);
	CN_ASSERT_NOT_NULL(config);

	if (!cnCommandLineParse_HasLookAhead(parse, 2)) {
		printf("Must provide the number of ticks for which to run the program.\n");
		return CN_ARG_PARSE_ERROR;
	}

	const char* tickString = cnCommandLineParse_LookAhead(parse, 2);
	char* readCursor;
	const int64_t parsedValue = strtoll(tickString, &readCursor, 10);
	if (parsedValue < 0) {
		printf("Cannot step a negative number of ticks: %s\n", tickString);
		return CN_ARG_PARSE_ERROR;
	}

	if (*readCursor != '\0' || errno == ERANGE) {
		printf("Unable to parse tick limit: %s\n", tickString);
		return CN_ARG_PARSE_ERROR;
	}
	config->tickLimit = parsedValue;
	return 2;
}

CnCommandLineParse cnCommandLineParse_Make(int argc, char** argv) {
	return (CnCommandLineParse) {
		.argc = argc,
		.argv = argv,

		// Start at index 1 to skip the program's name at index 0.
		.nextArgIndex = 1
	};
}

bool cnCommandLineParse_ShouldContinue(const CnCommandLineParse* parse)
{
	CN_ASSERT_NOT_NULL(parse);
	CN_ASSERT(parse->nextArgIndex > 0,
		"Next argument index out of bounds: %d", parse->nextArgIndex);
	CN_ASSERT(parse->nextArgIndex <= parse->argc,
		"Next argument index out of bounds: %d", parse->nextArgIndex);
	return parse->nextArgIndex < parse->argc;
}

bool cnCommandLineParse_IsComplete(const CnCommandLineParse* parse)
{
	CN_ASSERT_NOT_NULL(parse);
	return parse->nextArgIndex == parse->argc;
}

void cnCommandLineParse_Advance(CnCommandLineParse* parse, int argsParsed)
{
	CN_ASSERT_NOT_NULL(parse);
	parse->nextArgIndex += argsParsed;
}

/**
 * Return the index of the argument by looking forward from the current argument
 * position, with the next argument being at index 1, and the last parsed argument
 * being at index 0.
 */
int cnCommandLineParse_LookAheadIndex(const CnCommandLineParse* parse, int amount)
{
	CN_ASSERT_NOT_NULL(parse);
	CN_ASSERT(amount > 0, "Must look ahead at least by 1 argument.");
	const int index = parse->nextArgIndex + amount - 1;

	CN_ASSERT(index < parse->argc, "Look ahead out of range: %d", amount);
	return index;
}

const char* cnCommandLineParse_LookAhead(const CnCommandLineParse* parse, int amount)
{
	CN_ASSERT_NOT_NULL(parse);
	CN_ASSERT(amount > 0, "Must look ahead at least by 1 argument.");
	const int lookAheadIndex = cnCommandLineParse_LookAheadIndex(parse, amount);
	CN_ASSERT(lookAheadIndex > 0 || lookAheadIndex < parse->argc,
		"Look ahead out of bounds: %d out of %d total arguments.",
		lookAheadIndex, parse->argc);
	return parse->argv[lookAheadIndex];
}

/**
 * Look for a number of additional tokens, which includes the current argument.
 */
bool cnCommandLineParse_HasLookAhead(const CnCommandLineParse* parse, int amount)
{
	CN_ASSERT_NOT_NULL(parse);
	CN_ASSERT(amount > 0, "Must look ahead at least one argument: %d", amount);
	return cnCommandLineParse_LookAheadIndex(parse, amount) < parse->argc;
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
