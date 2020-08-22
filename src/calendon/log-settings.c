#include "log-settings.h"

CN_API char LogVerbosityChar[CnLogVerbosityNum] = {
	'F',
	'E',
	'W',
	'T'
};

CN_API const char* LogVerbosityString[CnLogVerbosityNum] = {
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
		if (LogVerbosityChar[i] == c) {
			*verbosity = i;
			return true;
		}
	}
	return false;
}