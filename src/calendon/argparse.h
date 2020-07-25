#ifndef CN_ARGPARSE_H
#define CN_ARGPARSE_H

/**
 * @file argparse.h
 *
 * Argument parsing function to a config.
 */

#include <calendon/cn.h>

#include <calendon/main-config.h>
#include <calendon/path.h>
#include <calendon/plugin.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A parser function which returns the number of arguments parsed.
 *
 * Should return `CN_ARG_PARSE_ERROR` if fails.
 */
typedef int32_t(*CnOptionParserFn)(int argc, char** argv, int i, CnMainConfig* config);

/**
 * An option, what it is, and how it should be parsed.
 */
typedef struct {
	const char* help;
	const char* shortOption;
	const char* longOption;
	CnOptionParserFn parser;
} CnCommandLineOption;

bool cnCommandLineOption_Matches(CnCommandLineOption* option, int argc, char** argv, int i);

extern CnCommandLineOption parsers[3];

void cnArgparse_PrintUsage(void);

/**
 * Return value if a parsing error occurred.
 */
#define CN_ARG_PARSE_ERROR -1

#ifdef __cplusplus
}
#endif

#endif /* CN_ARGPARSE_H */
