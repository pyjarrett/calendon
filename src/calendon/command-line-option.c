#include "command-line-option.h"

#include <calendon/env.h>
#include <calendon/string.h>

#include <calendon/main-config.h>

/**
 * Return if the next argument to parse matches the command line option.
 */
bool cnCommandLineOption_Matches(const CnCommandLineOption* option, const CnCommandLineParse* parse)
{
	CN_ASSERT_NOT_NULL(option);
	CN_ASSERT_NOT_NULL(parse->argv);
	CN_ASSERT(cnCommandLineParse_ShouldContinue(parse), "No more options to match.");
	const char* nextArg = cnCommandLineParse_LookAhead(parse, 1);
	return (option->shortOption && (strcmp(nextArg, option->shortOption) == 0)
			|| (option->longOption && (strcmp(nextArg, option->longOption) == 0)));
}
