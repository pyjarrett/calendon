#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <calendon/log-config.h>
#include <calendon/string.h>

CnLogHandle LogSysMain;

const char* g_logSystemNames[CN_LOG_MAX_SYSTEMS];

/**
 * Log functions don't work until the logging system is initialized, so that
 * needs to be tracked.
 */
static bool s_initialized = false;

/**
 * The memory system is not initialized before logging needs to be initialized.
 * Statically allocating this prevents any leaks, and ensure that it is ready
 * when the log system needs it.
 */
static char s_systemNameStorage[CN_LOG_MAX_SYSTEMS * CN_LOG_MAX_SYSTEM_NAME_TERMINATED_LENGTH];

// 4KiB is a lot of bytes.
CN_STATIC_ASSERT(sizeof(s_systemNameStorage) <= 4096,
	"Excessive storage used for system name storage in log system");

/**
 * Logging systems can only be reserved, and not freed.  Storage of system names
 * and verbosity settings past this value are considered invalid.
 */
static uint32_t s_numSystemsRegistered;

/**
 * Per-system verbosity settings given by values in `CN_LOG_*`.  Every system
 * has its own verbosity settings so you can disable the ones for spammy
 * systems while you ridicule their authors.
 */
static uint32_t s_systemVerbosities[CN_LOG_MAX_SYSTEMS];

/**
 * Provides a counter for every verbosity level so logging statistics can be
 * reported at program shutdown.  This quantifies how much log spam of each type
 * different systems give and the output could be checked for warnings.
 */
typedef struct {
	uint64_t counts[CnLogVerbosityNum];
} CnLogMessageCounter;

/**
 * Count the number of log messages produced by each system.
 */
static CnLogMessageCounter s_systemMessagesProduced[CN_LOG_MAX_SYSTEMS];

static CnLogConfig s_config;

void cnLogMessageCounter_Zero(CnLogMessageCounter* counter)
{
	memset(counter, 0, sizeof(*counter));
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
	return s_initialized;
}

/**
 * A specially called function to break the circular dependency exists between
 * command line parsing for log filters and the log system.
 */
void cnLog_PreInit(void)
{
#if _WIN32
	// Enable UTF-8 output on Windows.
	SetConsoleOutputCP(CP_UTF8);
#endif
	s_numSystemsRegistered = 0;

	for (int i=0; i<CN_LOG_MAX_SYSTEMS; ++i) {
		g_logSystemNames[i] = NULL;
	}

	s_initialized = true;

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
		cnPrint("    %*s", counterColumnWidth, g_logVerbosityString[i]);
	}
	cnPrint("\n");

	// Print message counts.
	for (uint32_t i = 0; i < s_numSystemsRegistered; ++i) {
		cnPrint("%*s", systemColumnWidth, g_logSystemNames[i]);
		for (uint32_t msgType = 0; msgType < CnLogVerbosityNum; ++msgType) {
			cnPrint("    %*" PRIu64, counterColumnWidth, s_systemMessagesProduced[i].counts[msgType]);
		}
		cnPrint("\n");
	}
}

void cnLog_Shutdown(void)
{
	CN_ASSERT(cnLog_IsReady(), "Cannot shutdown log system, it was never initialized.");

	cnLog_PrintStats();

	for (uint32_t i = 0; i < s_numSystemsRegistered; ++i) {
		g_logSystemNames[i] = NULL;
	}
	s_numSystemsRegistered = 0;
	s_initialized = false;
	CN_ASSERT(!cnLog_IsReady(), "Log system refused to shut down.");
}

/**
 * Registers a system for logging and get its name for display.
 *
 * Assumes "name" is somewhere where it doesn't need dynamic allocation.
 */
uint32_t cnLog_RegisterSystem(const char* name)
{
	CN_ASSERT_PTR(name);

	if (!cnLog_IsReady()) {
		CN_FATAL_ERROR("Log system not initialized, cannot register any systems.");
	}

	for (uint32_t i = 0; i < s_numSystemsRegistered; ++i) {
		if (cnString_Equal(name, g_logSystemNames[i], CN_LOG_MAX_SYSTEM_NAME_TERMINATED_LENGTH)) {
			return i;
		}
	}

	if (s_numSystemsRegistered + 1 >= CN_LOG_MAX_SYSTEMS) {
		CN_FATAL_ERROR("Cannot add any more systems to be logged; "
			"maximum number have already been registered.");
	}

	const CnLogHandle systemIndex = s_numSystemsRegistered++;
	char* nameStorage = &s_systemNameStorage[systemIndex * CN_LOG_MAX_SYSTEM_NAME_TERMINATED_LENGTH];
	cnString_Copy(nameStorage, name, CN_LOG_MAX_SYSTEM_NAME_TERMINATED_LENGTH);
	g_logSystemNames[systemIndex] = nameStorage;
	s_systemVerbosities[systemIndex] = CnLogVerbosityWarn;

	for (int i = 0; i < CnLogVerbosityNum; ++i) {
		s_systemMessagesProduced[systemIndex].counts[i] = 0;
	}

	return systemIndex;
}

CN_API void cnLog_SetVerbosity(CnLogHandle system, uint32_t verbosity)
{
	CN_ASSERT(system < s_numSystemsRegistered,
		"Cannot set verbosity for nonexistent system: " PRIu32, system);
	CN_ASSERT(cnLog_IsValidVerbosity(verbosity),
		"Verbosity setting is not valid: " PRIu32, verbosity);
	s_systemVerbosities[system] = verbosity;
}

void cnLog_Print(CnLogHandle system, CnLogVerbosity verbosity, const char* format, ...)
{
	++s_systemMessagesProduced[system].counts[verbosity];
	if (s_config.enabled && (uint32_t)verbosity <= s_systemVerbosities[system]) {
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
	}
}

CnBehavior cnLog_Plugin(void) {
	return (CnBehavior) {
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
