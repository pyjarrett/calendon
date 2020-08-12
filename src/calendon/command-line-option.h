#ifndef CN_COMMAND_LINE_OPTION_H
#define CN_COMMAND_LINE_OPTION_H

#include <calendon/cn.h>
#include <calendon/command-line-parse.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Indicates a parsing error occurs.
 */
enum { CnOptionParseError = -1 };

/**
 * A parser function which returns the number of arguments parsed.
 *
 * Should return `CnOptionParseError` if fails.
 */
typedef int32_t(*CnCommandLineOptionParser)(const CnCommandLineParse* parse, void* config);

/**
 * An option, what it is, and how it should be parsed.
 */
typedef struct {
	const char* help;
	const char* shortOption;
	const char* longOption;
	CnCommandLineOptionParser parser;
} CnCommandLineOption;

typedef struct {
	CnCommandLineOption* options;
	size_t numOptions;
} CnCommandLineOptionList;

bool cnCommandLineOption_Matches(const CnCommandLineOption* option, const CnCommandLineParse* parse);

#ifdef __cplusplus
}
#endif

#endif /* CN_COMMAND_LINE_OPTION_H */
