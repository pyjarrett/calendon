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
 * The current state of parsing command line arguments.
 */
typedef struct {
	int argc;
	char** argv;
	int nextArgIndex;
} CnCommandLineParse;

CnCommandLineParse cnCommandLineParse_Make(int argc, char** argv);
bool cnCommandLineParse_ShouldContinue(const CnCommandLineParse* parse);
bool cnCommandLineParse_IsComplete(const CnCommandLineParse* parse);
bool cnCommandLineParse_HasLookAhead(const CnCommandLineParse* parse, int amount);
const char* cnCommandLineParse_LookAhead(const CnCommandLineParse* parse, int amount);
int cnCommandLineParse_LookAheadIndex(const CnCommandLineParse* parse, int amount);
void cnCommandLineParse_Advance(CnCommandLineParse* parse, int argsParsed);

/**
 * A parser function which returns the number of arguments parsed.
 *
 * Should return `CnArgParseError` if fails.
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

/**
 * Return value if a parsing error occurred.
 */
enum { CnArgParseError = -1 };

#ifdef __cplusplus
}
#endif

#endif /* CN_ARGPARSE_H */
