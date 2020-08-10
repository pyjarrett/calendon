#include "argparse.h"

#include <calendon/cn.h>
#include <calendon/env.h>
#include <calendon/string.h>

#include <errno.h>
#include <string.h>

int32_t cnArgparse_TickLimit(const CnCommandLineParse* parse, CnMainConfig* config);

CnCommandLineOption parsers[] = {
	{
		"-t,--tick-limit NUM_TICKS  Limit the run to a specific number of ticks.\n",
		"-t",
		"--tick-limit",
		cnArgparse_TickLimit
	}
};

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

void cnArgparse_PrintUsage(int argc, char** argv)
{
	printf("Provided:\n");
	for (int i = 1; i < argc; ++i) {
		printf("%3d: \"%s\"\n", i, argv[i]);
	}

//	printf("\nUsage: calendon\n");
//	for (uint32_t i = 0; i < CN_ARRAY_SIZE(parsers); ++i) {
//		printf("%s", parsers[i].help);
//	}
	printf("\n");
}
