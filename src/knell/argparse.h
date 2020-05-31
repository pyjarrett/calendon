#ifndef KN_ARGPARSE_H
#define KN_ARGPARSE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file argparse.h
 *
 * Argument parsing function to a config.
 */

#include <knell/kn.h>
#include <knell/path.h>

typedef struct {
	PathBuffer gameLibPath;
	PathBuffer assetDirPath;
	int64_t tickLimit;
} knDriverConfig;

/**
 * A parser function which returns the number of arguments parsed.
 *
 * Should return `KN_ARG_PARSE_ERROR` if fails.
 */
typedef int32_t(* OptionParserFn)(int argc, char** argv, int i, knDriverConfig* config);

typedef struct {
	const char* help;
	const char* shortOption;
	const char* longOption;
	OptionParserFn parser;
} CommandParser;

/**
 * Return value if a parsing error occurred.
 */
#define KN_ARG_PARSE_ERROR -1

#ifdef __cplusplus
}
#endif

#endif /* KN_ARGPARSE_H */
