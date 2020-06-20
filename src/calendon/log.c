#include "log.h"

#include <stdio.h>

static bool initialized = false;
CN_API char LogVerbosityChar[CnLogVerbosityNum];
CN_API uint32_t LogSystemsVerbosity[CN_LOG_MAX_SYSTEMS];
CN_API const char* LogSystemsRegistered[CN_LOG_MAX_SYSTEMS];
CN_API uint32_t LogSystemsNumRegistered;
CN_API CnLogHandle LogSysMain;

bool cnLog_IsReady(void)
{
	return initialized;
}

void cnLog_Init(void)
{
#if _WIN32
	// Enable UTF-8 output on Windows.
	SetConsoleOutputCP(CP_UTF8);
#endif
	if (cnLog_IsReady()) {
		CN_FATAL_ERROR("Double initialization of logging system.");
	}
	LogSystemsNumRegistered = 0;

	for (int i=0; i<CN_LOG_MAX_SYSTEMS; ++i) {
		LogSystemsRegistered[i] = NULL;
	}

	LogVerbosityChar[CnLogVerbosityError] = 'E';
	LogVerbosityChar[CnLogVerbosityWarn] = 'W';
	LogVerbosityChar[CnLogVerbosityTrace] = 'T';

	initialized = true;

	cnLog_RegisterSystem(&LogSysMain, "Main", CnLogVerbosityTrace);
	CN_TRACE(LogSysMain, "Log system initialized.");
}

void cnLog_Shutdown(void)
{
	CN_ASSERT(cnLog_IsReady(), "Cannot shutdown log system, it was never initialized.");
	for (uint32_t i = 0; i < LogSystemsNumRegistered; ++i) {
		LogSystemsRegistered[i] = NULL;
	}
	LogSystemsNumRegistered = 0;
	initialized = false;
	CN_ASSERT(!cnLog_IsReady(), "Log system refused to shut down.");
}

/**
 * Registers a system for logging and get its name for display.
 *
 * Assumes "name" is somewhere where it doesn't need dynamic allocation.
 */
void cnLog_RegisterSystem(uint32_t* system, const char* name, uint32_t verbosity)
{
	if (!cnLog_IsReady()) {
		CN_FATAL_ERROR("Log system not initialized, cannot register any systems.");
	}

	// TODO: Is it possible to check the location of name to verify it's in the
	// program .rodata segment itself?
	if (!system) {
		CN_FATAL_ERROR("Cannot assign a log system ID to a null pointer.");
	}

	if (LogSystemsNumRegistered + 1 >= CN_LOG_MAX_SYSTEMS) {
		CN_FATAL_ERROR("Cannot add any more systems to be logged; "
			"maximum number have already been registered.");
	}

	*system = LogSystemsNumRegistered++;
	if (LogSystemsRegistered[*system] != NULL) {
		CN_FATAL_ERROR("Cannot register log system ID for '%s', ID is already used: %i\n",
			name, *system);
	}
	LogSystemsRegistered[*system] = name;
	LogSystemsVerbosity[*system] = verbosity;
}

