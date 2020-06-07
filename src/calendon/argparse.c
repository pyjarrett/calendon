#include "argparse.h"

#include <calendon/cn.h>
#include <calendon/env.h>

int32_t cnArgparse_Payload(int argc, char** argv, int index, CnMainConfig* config);
int32_t cnArgparse_AssetDir(int argc, char** argv, int index, CnMainConfig* config);
int32_t cnArgparse_TickLimit(int argc, char** argv, int index, CnMainConfig* config);

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

int32_t cnArgparse_Payload(int argc, char** argv, int index, CnMainConfig* config)
{
	CN_ASSERT(config, "Cannot parse payload to a null CnMainConfig.");
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

int32_t cnArgparse_AssetDir(int argc, char** argv, int index, CnMainConfig* config)
{
	CN_ASSERT(config, "Cannot parse asset dir to a null CnMainConfig.");
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

int32_t cnArgparse_TickLimit(int argc, char** argv, int index, CnMainConfig* config)
{
	CN_ASSERT(config, "Cannot parse tick limit a null CnMainConfig.");
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

void cnArgparse_PrintUsage(void)
{
	printf("\nUsage: calendon\n");
	for (uint32_t i = 0; i < CN_ARRAY_SIZE(parsers); ++i) {
		printf("%s", parsers[i].help);
	}
	printf("\n");
}
