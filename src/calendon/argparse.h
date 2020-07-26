#ifndef CN_ARGPARSE_H
#define CN_ARGPARSE_H

/**
 * @file argparse.h
 *
 * Argument parsing function to a config.
 */

#include <calendon/cn.h>

#include <calendon/command-line-parse.h>
#include <calendon/main-config.h>
#include <calendon/path.h>
#include <calendon/plugin.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Return value if a parsing error occurred.
 */
enum { CnOptionParseError = -1 };

/**
 * A parser function which returns the number of arguments parsed.
 *
 * Should return `CnOptionParseError` if fails.
 */
typedef int32_t(*CnOptionParserFn)(const CnCommandLineParse* parse, CnMainConfig* config);

/**
 * An option, what it is, and how it should be parsed.
 */
typedef struct {
	const char* help;
	const char* shortOption;
	const char* longOption;
	CnOptionParserFn parser;
} CnCommandLineOption;

bool cnCommandLineOption_Matches(const CnCommandLineOption* option, const CnCommandLineParse* parse);

extern CnCommandLineOption parsers[3];

void cnArgparse_PrintUsage(int argc, char** argv);

#ifdef __cplusplus
}
#endif

#endif /* CN_ARGPARSE_H */
