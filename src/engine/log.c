#include "log.h"

#include <stdio.h>

static bool initialized = false;
char LogVerbosityChar[KN_LOG_NUM_TYPES];
uint32_t LogSystemsVerbosity[KN_LOG_MAX_SYSTEMS];
const char* LogSystemsRegistered[KN_LOG_MAX_SYSTEMS];
uint32_t LogSystemsNumRegistered;
uint32_t LogSysMain;

void Log_Init()
{
	if (initialized) {
		KN_FATAL_ERROR("Double initialization of logging system.");
	}
	LogSystemsNumRegistered = 0;

	for (int i=0; i<KN_LOG_MAX_SYSTEMS; ++i) {
		LogSystemsRegistered[i] = NULL;
	}

	LogVerbosityChar[KN_LOG_ERROR] = 'E';
	LogVerbosityChar[KN_LOG_WARN] = 'W';
	LogVerbosityChar[KN_LOG_TRACE] = 'T';

	initialized = true;

	Log_RegisterSystem(&LogSysMain, "Main", KN_LOG_TRACE);
	KN_TRACE(LogSysMain, "Log system initialized.");
}

/**
 * Registers a system for logging and get its name for display.
 *
 * Assumes "name" is somewhere where it doesn't need dynamic allocation.
 */
void Log_RegisterSystem(uint32_t* system, const char* name, uint32_t verbosity)
{
	if (!initialized) {
		KN_FATAL_ERROR("Log system not initialized, cannot register any systems.");
	}

	// TODO: Is it possible to check the location of name to verify it's in the
	// program .rodata segment itself?
	if (!system) {
		KN_FATAL_ERROR("Cannot assign a log system ID to a null pointer.");
	}

	if (LogSystemsNumRegistered + 1 >= KN_LOG_MAX_SYSTEMS) {
		KN_FATAL_ERROR("Cannot add any more systems to be logged; "
			"maximum number have already been registered.");
	}

	*system = LogSystemsNumRegistered++;
	if (LogSystemsRegistered[*system] != NULL) {
		KN_FATAL_ERROR("Cannot register log system ID for '%s', ID is already used: %i\n",
			name, *system);
	}
	LogSystemsRegistered[*system] = name;
	LogSystemsVerbosity[*system] = verbosity;
}

