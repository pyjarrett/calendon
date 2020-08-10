#ifndef CN_ARGPARSE_H
#define CN_ARGPARSE_H

/**
 * @file argparse.h
 *
 * Argument parsing function to a config.
 */

#include <calendon/cn.h>

#include <calendon/command-line-option.h>
#include <calendon/command-line-parse.h>
#include <calendon/main-config.h>
#include <calendon/path.h>
#include <calendon/plugin.h>

#ifdef __cplusplus
extern "C" {
#endif

void cnArgparse_PrintUsage(int argc, char** argv);

#ifdef __cplusplus
}
#endif

#endif /* CN_ARGPARSE_H */
