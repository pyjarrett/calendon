#ifndef CN_ARGPARSE_H
#define CN_ARGPARSE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file argparse.h
 *
 * Argument parsing function to a config.
 */

#include <calendon/cn.h>
#include <calendon/path.h>
#include <calendon/plugin.h>

typedef struct {
	CnPlugin payload;

	CnPathBuffer gameLibPath;
	CnPathBuffer assetDirPath;

	int64_t tickLimit;
} CnDriverConfig;

/**
 * A parser function which returns the number of arguments parsed.
 *
 * Should return `CN_ARG_PARSE_ERROR` if fails.
 */
typedef int32_t(*CnOptionParserFn)(int argc, char** argv, int i, CnDriverConfig* config);

typedef struct {
	const char* help;
	const char* shortOption;
	const char* longOption;
	CnOptionParserFn parser;
} CnCommandParser;

/**
 * Return value if a parsing error occurred.
 */
#define CN_ARG_PARSE_ERROR -1

#ifdef __cplusplus
}
#endif

#endif /* CN_ARGPARSE_H */
