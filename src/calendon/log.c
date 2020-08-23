#include "log.h"

#include <stdarg.h>
#include <stdio.h>

#include <calendon/log-config.h>
#include <calendon/string.h>

static bool initialized = false;
CN_API uint32_t LogSystemsVerbosity[CN_LOG_MAX_SYSTEMS];
CN_API const char* LogSystemsRegistered[CN_LOG_MAX_SYSTEMS];
CN_API uint32_t LogSystemsNumRegistered;
CN_API CnLogMessageCounter LogMessagesProduced[CN_LOG_MAX_SYSTEMS];
CN_API CnLogHandle LogSysMain;

static char LogSystemNameStorage[CN_LOG_MAX_SYSTEMS * CN_LOG_MAX_SYSTEM_NAME_TERMINATED_LENGTH];
CN_STATIC_ASSERT(sizeof(LogSystemNameStorage) <= 4096,
	"Excessive storage used for system name storage in log system");

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

void cnLog_PreInit(void)
{
#if _WIN32
	// Enable UTF-8 output on Windows.
	SetConsoleOutputCP(CP_UTF8);
#endif
	LogSystemsNumRegistered = 0;

	for (int i=0; i<CN_LOG_MAX_SYSTEMS; ++i) {
		LogSystemsRegistered[i] = NULL;
	}

	initialized = true;

	LogSysMain = cnLog_RegisterSystem("Main");
}

bool cnLog_Init(void)
{
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
uint32_t cnLog_RegisterSystem(const char* name)
{
	CN_ASSERT_NOT_NULL(name);

	if (!cnLog_IsReady()) {
		CN_FATAL_ERROR("Log system not initialized, cannot register any systems.");
	}

	for (uint32_t i = 0; i < LogSystemsNumRegistered; ++i) {
		if (cnString_Equal(name, LogSystemsRegistered[i], CN_LOG_MAX_SYSTEM_NAME_TERMINATED_LENGTH)) {
			return i;
		}
	}

	if (LogSystemsNumRegistered + 1 >= CN_LOG_MAX_SYSTEMS) {
		CN_FATAL_ERROR("Cannot add any more systems to be logged; "
			"maximum number have already been registered.");
	}

	const CnLogHandle systemIndex = LogSystemsNumRegistered++;
	char* nameStorage = &LogSystemNameStorage[systemIndex * CN_LOG_MAX_SYSTEM_NAME_TERMINATED_LENGTH];
	cnString_Copy(nameStorage, name, CN_LOG_MAX_SYSTEM_NAME_TERMINATED_LENGTH);
	LogSystemsRegistered[systemIndex] = nameStorage;
	LogSystemsVerbosity[systemIndex] = CnLogVerbosityWarn;

	for (int i = 0; i < CnLogVerbosityNum; ++i) {
		LogMessagesProduced[systemIndex].counts[i] = 0;
	}

	return systemIndex;
}

CN_API void cnLogHandle_SetVerbosity(CnLogHandle system, uint32_t verbosity)
{
	CN_ASSERT(cnLog_IsValidVerbosity(verbosity),
		"Verbosity setting is not valid: " PRIu32, verbosity);
	LogSystemsVerbosity[system] = verbosity;
}

void cnLog_Print(CnLogHandle system, CnLogVerbosity verbosity, const char* format, ...)
{
	++LogMessagesProduced[system].counts[verbosity];
	if (s_config.enabled && (uint32_t)verbosity <= LogSystemsVerbosity[system]) {
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
