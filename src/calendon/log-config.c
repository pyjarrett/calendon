#include "log-config.h"

#include <calendon/command-line-option.h>
#include <calendon/command-line-parse.h>
#include <calendon/log.h>
#include <calendon/string.h>

static CnLogConfig s_logConfig;

int32_t cnLog_OptionDisable(const CnCommandLineParse* parse, void* config)
{
	CnLogConfig* c = (CnLogConfig*)config;
	c->enabled = false;
	return 1;
}

int32_t cnLog_OptionFilter(const CnCommandLineParse* parse, void* config)
{
	CN_ASSERT_NOT_NULL(parse);
	CN_UNUSED(config);

	int argsParsed = 1;
	while (cnCommandLineParse_HasLookAhead(parse, argsParsed + 1)) {
		const char* nextArg = cnCommandLineParse_LookAhead(parse, argsParsed + 1);

		// A reasonable bounds placed on argument length.
		const int maxArgLength = 65536;

		size_t argNumChars;
		if (!cnString_NumCharacterBytes(nextArg, maxArgLength, &argNumChars)) {
			cnPrint("Next argument exceeded maximum allowed argument length.\n");
			return CnOptionParseError;
		}

		if (cnString_HasPrefix(nextArg, maxArgLength, "-")
			|| cnString_HasPrefix(nextArg, maxArgLength, "--"))
		{
			if (argsParsed == 1) {
				cnPrint("No log filters provided.\n");
				return CnOptionParseError;
			}
			return argsParsed;
		}

		// Ensure the string is at least 3 characters for system:verbosity.
		if (argNumChars < 3) {
			return CnOptionParseError;
		}

		size_t delimiterIndex;
		if (!cnString_FirstIndexOfChar(nextArg, maxArgLength, ':', &delimiterIndex)) {
			return CnOptionParseError;
		}
		CN_ASSERT(delimiterIndex < argNumChars, "Delimiter index %zu is out of bounds: %zu",
			delimiterIndex, argNumChars);

		if (delimiterIndex == argNumChars) {
			cnPrint("No verbosity setting provided: %s\n", nextArg);
			return CnOptionParseError;
		}

		const size_t systemNameNumBytes = delimiterIndex;
		if (argNumChars - 1 - systemNameNumBytes != 1) {
			cnPrint("Verbosity settings must be single characters.\n");
			return CnOptionParseError;
		}

		char systemName[CN_LOG_MAX_SYSTEM_NAME_TERMINATED_LENGTH];
		if (argNumChars >= CN_LOG_MAX_SYSTEM_NAME_TERMINATED_LENGTH) {
			return CnOptionParseError;
		}
		memset(systemName, 0, CN_LOG_MAX_SYSTEM_NAME_TERMINATED_LENGTH);
		memcpy(systemName, nextArg, systemNameNumBytes);

		const char verbosityChar = nextArg[delimiterIndex + 1];
		uint32_t verbosity;
		if (!cnLog_VerbosityFromChar(verbosityChar, &verbosity)) {
			cnPrint("Unknown verbosity setting: %c", verbosityChar);
			return CnOptionParseError;
		}

		CnLogHandle systemLogHandle = cnLog_RegisterSystem(systemName);
		cnLog_SetVerbosity(systemLogHandle, verbosity);

		argsParsed += 1;
	}

	if (argsParsed == 1) {
		cnPrint("No log filters provided.\n");
		return CnOptionParseError;
	}
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
			"\t\tModify the verbosity setting for a system.  Multiple SYSTEM:VERBOSITY\n"
			"\t\tpairs can be used in sequence or the entire --log-filter flag can be\n"
			"\t\trepeated.\n"
			"\t\t\tVerbosity settings:\n"
			"\t\t\tT: trace\n"
			"\t\t\tW: warnings\n"
			"\t\t\tE: errors\n"
			"\n"
			"\t\tExamples:\n"
			"\t\t\t--log-filter Main:E      Only show errors in main.\n"
			"\t\t\t--log-filter Render:W    Show warnings/errors in the renderer.\n"
			"\t\t\t--log-filter Render:T    Show all messages in the renderer.\n",
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
