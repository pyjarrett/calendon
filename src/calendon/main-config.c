#include "main-config.h"

#include <calendon/cn.h>
#include <calendon/argparse.h>

#include <string.h>


bool cnMainConfig_IsHosted(CnMainConfig* config)
{
	CN_ASSERT(config, "Cannot determine if null config is hosted.");
	return cnPlugin_IsComplete(&config->payload);
}

/**
 * Parses a set of arguments into a driver configuration.
 */
bool cnMainConfig_ParseCommandLine(CnMainConfig* config, int argc, char** argv)
{
	CN_ASSERT(argc >= 1, "Argument count must at least include the executable.");
	CN_ASSERT(argv, "Cannot parse null arguments.");
	CN_ASSERT(config, "Cannot parse arguments to a null CnMainConfig.");

	cnPathBuffer_Clear(&config->gameLibPath);
	cnPathBuffer_Clear(&config->assetDirPath);

	memset(&config->payload, 0, sizeof(config->payload));
	config->tickLimit = 0;

	CnCommandLineParse commandLineParse = cnCommandLineParse_Make(argc, argv);

	// The log system is not initialized at this point, so use printf.
	while (cnCommandLineParse_ShouldContinue(&commandLineParse)) {
		bool parseAdvanced = false;
		for (uint32_t parserIndex = 0; parserIndex < CN_ARRAY_SIZE(parsers); ++parserIndex) {
			if (cnCommandLineOption_Matches(&parsers[parserIndex], &commandLineParse)) {
				const int32_t argsParsed = parsers[parserIndex].parser(&commandLineParse, config);
				if (argsParsed == CN_ARG_PARSE_ERROR) {
					cnArgparse_PrintUsage(argc, argv);
					return false;
				}
				cnCommandLineParse_Advance(&commandLineParse, argsParsed);
				parseAdvanced = true;
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

void cnMainConfig_Freestanding(CnMainConfig* config, CnPlugin_InitFn init,
	CnPlugin_TickFn tick, CnPlugin_DrawFn draw, CnPlugin_ShutdownFn shutdown)
{
	CN_ASSERT(config, "Cannot create a freestanding implementation from a null config.");

	config->payload.init = init;
	config->payload.tick = tick;
	config->payload.draw = draw;
	config->payload.shutdown = shutdown;
	config->payload.sharedLibrary = NULL;

	if (!cnMainConfig_IsHosted(config)) {
		CN_FATAL_ERROR("Improperly hosted config.");
	}
}
