#include "argparse.h"

#include <calendon/cn.h>
#include <calendon/env.h>
#include <calendon/string.h>

#include <errno.h>
#include <string.h>

void cnArgparse_PrintUsage(int argc, char** argv)
{
	cnPrint("Provided:\n");
	for (int i = 1; i < argc; ++i) {
		cnPrint("%3d: \"%s\"\n", i, argv[i]);
	}

//	printf("\nUsage: calendon\n");
//	for (uint32_t i = 0; i < CN_ARRAY_SIZE(parsers); ++i) {
//		printf("%s", parsers[i].help);
//	}
	cnPrint("\n");
}
