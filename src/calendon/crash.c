#include "crash.h"

#include <calendon/log.h>

#include <stdio.h>
#include <string.h>

#ifdef _WIN32

#if CN_ENABLE_CORE_DUMPS
#include <calendon/path.h>
#include <calendon/compat-windows.h>
#include <DbgHelp.h>

/**
 * Opens a unique file for writing a minidump in the same directory as the
 * executable so it's easy to find.
 */
HANDLE cnCrash_OpenMiniDumpFile(DWORD processId)
{
	CnPathBuffer miniDumpFilePath;
	if (!cnPathBuffer_CurrentWorkingDirectory(&miniDumpFilePath)) {
		// Logging will probably not work during a crash.  Try anyways.
		CN_ERROR(LogSysMain, "Unable to get current working directory when "
			"making MiniDump file.");
		return INVALID_HANDLE_VALUE;
	}

	char miniDumpFileName[128];
	cnString_Format(miniDumpFileName, 128, "core.%lu.dmp", processId);
	if (!cnPathBuffer_Join(&miniDumpFilePath, miniDumpFileName)) {
		CN_ERROR(LogSysMain, "Unable to build minidump file path");
		return INVALID_HANDLE_VALUE;
	}
	CN_TRACE(LogSysMain, "MiniDumpFilePath is %s", miniDumpFilePath.str);

	HANDLE hFile = CreateFile(miniDumpFilePath.str, GENERIC_WRITE, FILE_SHARE_READ,
		0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE) {
		CN_ERROR(LogSysMain, "Unable to open file for minidump: %s", miniDumpFileName);
	}
	return hFile;
}

/**
 * A unique crash handler to be installed to generate the minidump.
 */
// http://ntcoder.com/bab/2014/10/14/how-to-create-full-memory-dumps-using-minidumpwritedump/
// https://stackoverflow.com/questions/5028781/how-to-write-a-sample-code-that-will-crash-and-produce-dump-file
LONG CALLBACK cnCrash_Handler(EXCEPTION_POINTERS* exceptionPointers)
{
	CN_UNUSED(exceptionPointers);

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
	HANDLE hFile = cnCrash_OpenMiniDumpFile(processId);

	MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
	exceptionInfo.ThreadId = GetCurrentThreadId();
	exceptionInfo.ExceptionPointers = exceptionPointers;
	exceptionInfo.ClientPointers = FALSE;
	MiniDumpWriteDump(hProcess, processId, hFile, flags,
		exceptionPointers ? &exceptionInfo : NULL, NULL, NULL);
	CloseHandle(hFile);
	return EXCEPTION_CONTINUE_SEARCH;
}

bool cnCrash_EnableCoreDump(void)
{
	SetUnhandledExceptionFilter(cnCrash_Handler);
	return true;
}
#endif /* CN_ENABLE_CORE_DUMPS */

bool cnCrash_Init(void)
{
#if CN_ENABLE_CORE_DUMPS
	if (!cnCrash_EnableCoreDump()) {
		CN_WARN(LogSysMain, "Unable to enable core dumps");
	}
#endif
	return true;
}
#endif

#ifdef __linux__

#include <sys/resource.h>
#include <errno.h>

/**
 * The OS determines the placement of the core files with core_pattern.  This
 * might be set to "|/bin/false" which causes no core dump to be created.
 */
void cnCrash_PrintCoreDumpPattern(void)
{
	FILE* corePatternFile = fopen("/proc/sys/kernel/core_pattern", "r");
	char buffer[4096];
	size_t amountRead = fread(buffer, 1, 4096, corePatternFile);
	if (amountRead > 0) {
		if (strcmp(buffer, "|/bin/false\n") == 0) {
			CN_WARN(LogSysMain, "Set /proc/sys/kernel/core_pattern to "
				"generate core dumps.");
			CN_WARN(LogSysMain, "You might be able to modify it with "
				"`sudo sysctl -w kernel.core_pattern=core.%%e.%%p`");
		}
		CN_TRACE(LogSysMain, "Core dump pattern: '%s'", buffer);
	}
	fclose(corePatternFile);
}

/**
 * Changes ulimit to maximum size, provided it is allowed by parent process.
 */
bool cnCrash_EnableCoreDump(void)
{
	struct rlimit newLimit;
	newLimit.rlim_cur = RLIM_INFINITY;
	newLimit.rlim_max = RLIM_INFINITY;
	if (setrlimit(RLIMIT_CORE, &newLimit) != 0) {
		switch (errno) {
			case EFAULT:
				CN_ERROR(LogSysMain, "Bad pointer fed to setrlimit.");
				break;
			case EINVAL:
				CN_ERROR(LogSysMain, "Invalid value given to setrlimit %lu (max: %lu)",
					newLimit.rlim_cur, newLimit.rlim_max);
				break;
			case EPERM:
				CN_ERROR(LogSysMain, "Invalid permissions to set setrlimit");
				break;
			default:
				CN_ERROR(LogSysMain, "Unable to set rlimit.");
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
		CN_TRACE(LogSysMain, "Tried to set core dump limit to %lu, but is %lu"
				"(max: %lu)", newLimit.rlim_cur, currentLimit.rlim_cur,
				currentLimit.rlim_max);
		return false;
	}
}

bool cnCrash_Init(void)
{
#if CN_ENABLE_CORE_DUMPS
	if (!cnCrash_EnableCoreDump()) {
		CN_WARN(LogSysMain, "Unable to enable core dumps");
	}
	cnCrash_PrintCoreDumpPattern();
#endif
	return true;
}

#endif

CnPlugin cnCrash_Plugin(void)
{
	return (CnPlugin) {
		.init = cnCrash_Init,
		.shutdown = NULL,
		.tick = NULL,
		.draw = NULL,
		.sharedLibrary = NULL
	};
}

CnSystem cnCrash_System(void)
{
	return (CnSystem) {
		.name = "Crash",
		.plugin = cnCrash_Plugin,
		.options = cnSystem_NoOptions,
		.config = cnSystem_NoConfig,
		.setDefaultConfig = cnSystem_NoDefaultConfig
	};
}
