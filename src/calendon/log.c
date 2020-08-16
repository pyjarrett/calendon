#include "log.h"

#include <stdarg.h>
#include <stdio.h>

#include <calendon/log-config.h>

static bool initialized = false;
CN_API char LogVerbosityChar[CnLogVerbosityNum];
CN_API const char* LogVerbosityString[CnLogVerbosityNum];
CN_API uint32_t LogSystemsVerbosity[CN_LOG_MAX_SYSTEMS];
CN_API const char* LogSystemsRegistered[CN_LOG_MAX_SYSTEMS];
CN_API uint32_t LogSystemsNumRegistered;
CN_API CnLogMessageCounter LogMessagesProduced[CN_LOG_MAX_SYSTEMS];
CN_API CnLogHandle LogSysMain;

static CnLogConfig s_config;

void cnLogMessageCounter_Zero(CnLogMessageCounter* counter)
{
	memset(counter, 0, sizeof(counter));
}

void* cnLog_Config(void) {
	return &s_config;
}

void cnLog_SetDefaultConfig(void* config)
{
	CnLogConfig* c = (CnLogConfig*)config;
	c->enabled = true;
}

bool cnLog_IsReady(void)
{
	return initialized;
}

bool cnLog_Init(void)
{
#if _WIN32
	// Enable UTF-8 output on Windows.
	SetConsoleOutputCP(CP_UTF8);
#endif
	if (cnLog_IsReady()) {
		CN_FATAL_ERROR("Double initialization of logging system.");
		return false;
	}
	LogSystemsNumRegistered = 0;

	for (int i=0; i<CN_LOG_MAX_SYSTEMS; ++i) {
		LogSystemsRegistered[i] = NULL;
	}

	LogVerbosityChar[CnLogVerbosityFatal] = 'F';
	LogVerbosityChar[CnLogVerbosityError] = 'E';
	LogVerbosityChar[CnLogVerbosityWarn] = 'W';
	LogVerbosityChar[CnLogVerbosityTrace] = 'T';

	LogVerbosityString[CnLogVerbosityFatal] = "Fatal";
	LogVerbosityString[CnLogVerbosityError] = "Error";
	LogVerbosityString[CnLogVerbosityWarn] = "Warning";
	LogVerbosityString[CnLogVerbosityTrace] = "Trace";

	initialized = true;

	cnLog_RegisterSystem(&LogSysMain, "Main", CnLogVerbosityTrace);
	CN_TRACE(LogSysMain, "Log system initialized.");
	return true;
}

void cnLog_PrintStats(void)
{
	CN_ASSERT(cnLog_IsReady(), "Cannot shutdown log system, it was never initialized.");

	const int systemColumnWidth = 30;
	const int counterColumnWidth = 16;

	cnPrint("\nTotal log messages written\n");

	// Print column headers
	cnPrint("%*s", systemColumnWidth, "");
	for (uint32_t i = 0; i < CnLogVerbosityNum; ++i) {
		cnPrint("    %*s", counterColumnWidth, LogVerbosityString[i]);
	}
	cnPrint("\n");

	// Print message counts.
	for (uint32_t i = 0; i < LogSystemsNumRegistered; ++i) {
		cnPrint("%*s", systemColumnWidth, LogSystemsRegistered[i]);
		for (uint32_t msgType = 0; msgType < CnLogVerbosityNum; ++msgType) {
			cnPrint("    %*" PRIu64, counterColumnWidth, LogMessagesProduced[i].counts[msgType]);
		}
		cnPrint("\n");
	}
}

void cnLog_Shutdown(void)
{
	CN_ASSERT(cnLog_IsReady(), "Cannot shutdown log system, it was never initialized.");

	cnLog_PrintStats();

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

	for (int i = 0; i < CnLogVerbosityNum; ++i) {
		LogMessagesProduced[*system].counts[i] = 0;
	}
}

void cnLog_Print(CnLogHandle system, CnLogVerbosity verbosity, const char* format, ...)
{
	if (s_config.enabled && (uint32_t)verbosity <= LogSystemsVerbosity[system]) {
		++LogMessagesProduced[system].counts[verbosity];

		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
	}
}

CnPlugin cnLog_Plugin(void) {
	return (CnPlugin) {
		.init = cnLog_Init,
		.shutdown = cnLog_Shutdown,
		.tick = NULL,
		.draw = NULL,
		.sharedLibrary = NULL
	};
}

CnSystem cnLog_System(void)
{
	return (CnSystem) {
		.name = "Log",
		.options = cnLog_CommandLineOptionList,
		.config = cnLog_Config,
		.setDefaultConfig = cnLog_SetDefaultConfig,
		.plugin = cnLog_Plugin
	};
}
