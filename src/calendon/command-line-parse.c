#include "command-line-parse.h"

/**
 * Starts a parse of the command line at the first argument.
 */
CnCommandLineParse cnCommandLineParse_Make(int argc, char** argv) {
	CN_ASSERT(argc > 0, "A command line must have at least the program name.");
	CN_ASSERT_NOT_NULL(argv);

	return (CnCommandLineParse) {
		.argc = argc,
		.argv = argv,

		// Start at index 1 to skip the program's name at index 0.
		.numArgsParsed = 1
	};
}

/**
 * The parse should continue if there are unparsed arguments.
 */
bool cnCommandLineParse_ShouldContinue(const CnCommandLineParse* parse)
{
	CN_ASSERT_NOT_NULL(parse);
	CN_ASSERT(parse->numArgsParsed > 0,
		"Next argument index out of bounds: %d", parse->numArgsParsed);
	CN_ASSERT(parse->numArgsParsed <= parse->argc,
		"Next argument index out of bounds: %d", parse->numArgsParsed);
	return parse->numArgsParsed < parse->argc;
}

bool cnCommandLineParse_IsComplete(const CnCommandLineParse* parse)
{
	CN_ASSERT_NOT_NULL(parse);
	return parse->numArgsParsed == parse->argc;
}

/**
 * Moves the parse ahead a specific number of arguments.
 */
void cnCommandLineParse_Advance(CnCommandLineParse* parse, int argsParsed)
{
	CN_ASSERT_NOT_NULL(parse);
	parse->numArgsParsed += argsParsed;
	CN_ASSERT(parse->numArgsParsed <= parse->argc, "Advanced out of parser range to %d by %d",
			  parse->argc, argsParsed);
}

/**
 * Return the index of the argument by looking forward from the current argument
 * position, with the next argument being at index 1, and the last parsed argument
 * being at index 0.
 */
int cnCommandLineParse_LookAheadIndex(const CnCommandLineParse* parse, int amount)
{
	CN_ASSERT_NOT_NULL(parse);
	CN_ASSERT(amount > 0, "Must look ahead at least by 1 argument.");
	return (parse->numArgsParsed - 1) + amount;
}

/**
 * The argument contents found by looking ahead a specific number of arguments
 * ahead of the already parsed arguments.
 */
const char* cnCommandLineParse_LookAhead(const CnCommandLineParse* parse, int amount)
{
	CN_ASSERT_NOT_NULL(parse);
	CN_ASSERT(amount > 0, "Must look ahead at least by 1 argument.");
	const int lookAheadIndex = cnCommandLineParse_LookAheadIndex(parse, amount);
	CN_ASSERT(lookAheadIndex > 0 || lookAheadIndex < parse->argc,
		"Look ahead out of bounds: %d out of %d total arguments.",
		lookAheadIndex, parse->argc);
	return parse->argv[lookAheadIndex];
}

/**
 * Look for a number of additional tokens, which includes the current argument.
 */
bool cnCommandLineParse_HasLookAhead(const CnCommandLineParse* parse, int amount)
{
	CN_ASSERT_NOT_NULL(parse);
	CN_ASSERT(amount > 0, "Must look ahead at least one argument: %d", amount);
	return cnCommandLineParse_LookAheadIndex(parse, amount) < parse->argc;
}
