#include "log-config.h"

#include <calendon/command-line-option.h>
#include <calendon/command-line-parse.h>

static CnLogConfig s_logConfig;

int32_t cnLog_OptionDisable(const CnCommandLineParse* parse, void* config)
{
	CnLogConfig* c = (CnLogConfig*)config;
	c->enabled = false;
	return 1;
}

int32_t cnLog_OptionFilter(const CnCommandLineParse* parse, void* config)
{
	CnLogConfig* c = (CnLogConfig*)config;

	int argsParsed = 1;
	while (cnCommandLineParse_HasLookAhead(parse, argsParsed + 1)) {
		// Ensure the string has non-zero length.
		// Ensure the string is at least 3 characters for system:verbosity.
		// Ensure the value isn't prefixed with "-"
		// Attempt to parse the next value.
		// Find the location of the split.
		// Set all verbosity settings if the system is "*".
		// Ensure the system to set verbosity for actually exists.
		// Determine the verbosity type.
		// Check the verbosity string is only one character.
			// Report an error for invalid verbosity.
		// TODO: call register system automatically after systems are created.
		// TODO: manually initialize the basics of the logging system before other systems.
	}

	CN_FATAL_ERROR("NOT DONE");
	return argsParsed;
}

static CnCommandLineOption options[] = {
	{
		"\t--log-disable\n"
			"\t\tDisables ALL runtime logging output.\n",
		NULL,
		"--log-disable",
		cnLog_OptionDisable,
	},
	{
		"\t--log-filter SYSTEM:VERBOSITY\n"
			"\t\tModify the verbosity setting for a system.  Multiple SYSTEM:VERBOSITY"
			"\t\tpairs can be used in sequence or the entire --log-filter flag can be"
			"\t\trepeated.\n"
			"\t\t\tVerbosity settings:"
			"\t\t\tT: trace\n"
			"\t\t\tW: warnings\n"
			"\t\t\tE: errors\n"
			"\n"
			"\t\tExamples:"
			"\t\t\t--log-filter Main:E      Only show errors in main."
			"\t\t\t--log-filter Render:W    Show warnings/errors in the renderer."
			"\t\t\t--log-filter Render:T    Show all messages in the renderer.",
			NULL,
			"--log-filter",
			cnLog_OptionFilter,
	}
};

CnCommandLineOptionList cnLog_CommandLineOptionList(void) {
	return (CnCommandLineOptionList) {
		.options = options,
		.numOptions = 2
	};
}
