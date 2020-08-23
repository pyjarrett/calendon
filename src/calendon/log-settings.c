#include "log-settings.h"

char g_logVerbosityChar[CnLogVerbosityNum] = {
	'F',
	'E',
	'W',
	'T'
};

const char* g_logVerbosityString[CnLogVerbosityNum] = {
	"Fatal",
	"Error",
	"Warning",
	"Trace"
};

bool cnLog_IsValidVerbosity(uint32_t verbosity)
{
	return verbosity < CnLogVerbosityNum;
}

bool cnLog_VerbosityFromChar(char c, uint32_t* verbosity)
{
	for (uint32_t i = 0; i < CnLogVerbosityNum; ++i) {
		if (g_logVerbosityChar[i] == c) {
			*verbosity = i;
			return true;
		}
	}
	return false;
}