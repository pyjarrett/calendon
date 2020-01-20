#include "crash.h"

#include "log.h"

#include <string.h>

#ifdef _WIN32

bool Crash_EnableCoreDump(void)
{
	// TODO: Implement with MiniDumpWriteDump and SetUnhandledExceptionFilter
	// MiniDumpWriteDump
	// SetUnhandledExceptionFilter
	return false;
}

KN_API void Crash_Init(void)
{
	if (!Crash_EnableCoreDump()) {
		KN_WARN(LogSysMain, "Unable to enable core dumps");
	}
}
#endif

#ifdef __linux__

#include <sys/resource.h>
#include <errno.h>

/**
 * The OS determines the placement of the core files with core_pattern.  This
 * might be set to "|/bin/false" which causes no core dump to be created.
 */
void Crash_PrintCoreDumpPattern(void)
{
	FILE* corePatternFile = fopen("/proc/sys/kernel/core_pattern", "r");
	char buffer[4096];
	size_t amountRead = fread(buffer, 1, 4096, corePatternFile);
	if (amountRead > 0) {
		if (strcmp(buffer, "|/bin/false\n") == 0) {
			KN_WARN(LogSysMain, "Set /proc/sys/kernel/core_pattern to "
				"generate core dumps.");
			KN_WARN(LogSysMain, "You might be able to modify it with "
				"`sudo sysctl -w kernel.core_pattern=core.%%e.%%p`");
		}
		KN_TRACE(LogSysMain, "Core dump pattern: '%s'", buffer);
	}
	fclose(corePatternFile);
}

/**
 * Changes ulimit to maximum size, provided it is allowed by parent process.
 */
bool Crash_EnableCoreDump(void)
{
	struct rlimit newLimit;
	newLimit.rlim_cur = RLIM_INFINITY;
	newLimit.rlim_max = RLIM_INFINITY;
	if (setrlimit(RLIMIT_CORE, &newLimit) != 0) {
		switch (errno) {
			case EFAULT:
				KN_ERROR(LogSysMain, "Bad pointer fed to setrlimit.");
				break;
			case EINVAL:
				KN_ERROR(LogSysMain, "Invalid value given to setrlimit %lu (max: %lu)",
					newLimit.rlim_cur, newLimit.rlim_max);
				break;
			case EPERM:
				KN_ERROR(LogSysMain, "Invalid permissions to set setrlimit");
				break;
			default:
				KN_ERROR(LogSysMain, "Unable to set rlimit.");
				break;
		}
		return false;
	}

	struct rlimit currentLimit;
	getrlimit(RLIMIT_CORE, &currentLimit);
	if (newLimit.rlim_cur == currentLimit.rlim_cur) {
		return true;
	}
	else {
		KN_TRACE(LogSysMain, "Tried to set core dump limit to %lu, but is %lu"
				"(max: %lu)", newLimit.rlim_cur, currentLimit.rlim_cur,
				currentLimit.rlim_max);
		return false;
	}
}

KN_API void Crash_Init(void)
{
#if KN_ENABLE_CORE_DUMPS
	if (!Crash_EnableCoreDump()) {
		KN_WARN(LogSysMain, "Unable to enable core dumps");
	}
	Crash_PrintCoreDumpPattern();
#endif
}

#endif
