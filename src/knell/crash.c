#include "crash.h"

#include "log.h"

#include <string.h>

#ifdef _WIN32

#if KN_ENABLE_CORE_DUMPS
#include "env.h"
#include "path.h"
#include "compat-windows.h"
#include "DbgHelp.h"

/**
 * Opens a unique file for writing a minidump in the same directory as the
 * executable so it's easy to find.
 */
HANDLE Crash_OpenMiniDumpFile(DWORD processId)
{
	char miniDumpFilePath[MAX_PATH];
	if (!Env_CurrentWorkingDirectory(miniDumpFilePath, MAX_PATH)) {
		// Logging will probably not work during a crash.  Try anyways.
		KN_ERROR(LogSysMain, "Unable to find current working directory when"
			"making MiniDump file.");
	}
	char miniDumpFileName[1024];
	snprintf(miniDumpFileName, 1024, "core.%lu.dmp", processId);
	//const char* miniDumpFileName = "core.dump";
	if (!Path_Append(miniDumpFileName, miniDumpFilePath, MAX_PATH)) {
		KN_ERROR(LogSysMain, "Unable to build minidump file path");
		return INVALID_HANDLE_VALUE;
	}
	KN_TRACE(LogSysMain, "MiniDumpFilePath is %s", miniDumpFilePath);

	HANDLE hFile = CreateFile(miniDumpFilePath, GENERIC_WRITE, FILE_SHARE_READ,
		0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE) {
		KN_ERROR(LogSysMain, "Unable to open file for minidump: %s", miniDumpFileName);
	}
	return hFile;
}

/**
 * A unique crash handler to be installed to generate the minidump.
 */
// http://ntcoder.com/bab/2014/10/14/how-to-create-full-memory-dumps-using-minidumpwritedump/
// https://stackoverflow.com/questions/5028781/how-to-write-a-sample-code-that-will-crash-and-produce-dump-file
LONG CALLBACK Crash_Handler(EXCEPTION_POINTERS* exceptionPointers)
{
	KN_UNUSED(exceptionPointers);

	DWORD processId = GetCurrentProcessId();
	HANDLE hProcess;
	hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
							FALSE, processId );

	const DWORD flags = MiniDumpNormal;
// These options lead to massive (150-250 MiB), but very informative minidumps.
// Leaving here for reference.
// Minidumps when this was written with only MiniDumpNormal are ~86 KiB.
//						MiniDumpWithFullMemory
//						| MiniDumpWithFullMemoryInfo
//						| MiniDumpWithHandleData
//						| MiniDumpWithUnloadedModules
//						| MiniDumpWithThreadInfo;
	HANDLE hFile = Crash_OpenMiniDumpFile(processId);

	MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
	exceptionInfo.ThreadId = GetCurrentThreadId();
	exceptionInfo.ExceptionPointers = exceptionPointers;
	exceptionInfo.ClientPointers = FALSE;
	MiniDumpWriteDump(hProcess, processId, hFile, flags,
		exceptionPointers ? &exceptionInfo : NULL, NULL, NULL);
	CloseHandle(hFile);
	return EXCEPTION_CONTINUE_SEARCH;
}

bool Crash_EnableCoreDump(void)
{
	SetUnhandledExceptionFilter(Crash_Handler);
	return true;
}
#endif /* KN_ENABLE_CORE_DUMPS */

KN_API void Crash_Init(void)
{
#if KN_ENABLE_CORE_DUMPS
	if (!Crash_EnableCoreDump()) {
		KN_WARN(LogSysMain, "Unable to enable core dumps");
	}
#endif
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
