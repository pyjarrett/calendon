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

static CnCommandLineOption options[] = {
	{
		"\t--log-disable\n"
			"\t\tDisables runtime logging output.\n",
		NULL,
		"--log-disable",
		cnLog_OptionDisable,
	}
};

CnCommandLineOptionList cnLog_CommandLineOptionList(void) {
	return (CnCommandLineOptionList) {
		.options = options,
		.numOptions = 1
	};
}
