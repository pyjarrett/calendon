#include "process.h"

#include <knell/log.h>

#ifdef _WIN32

#include <knell/compat-windows.h>
#include <tchar.h>
#include <psapi.h>

// Adapted from sample code at:
// https://docs.microsoft.com/en-us/windows/win32/psapi/enumerating-all-modules-for-a-process?redirectedfrom=MSDN
bool Proc_PrintLoadedDLLs(void)
{
	DWORD processID = GetCurrentProcessId();
	HANDLE hProcess;

	KN_TRACE(LogSysMain, "Process ID: %lu", processID);

	hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
							PROCESS_VM_READ,
							FALSE, processID );
	if (hProcess == NULL) {
		return false;
	}

	HMODULE hMods[1024];
	DWORD cbNeeded;
	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
		for (uint64_t i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i)
		{
			TCHAR szModName[MAX_PATH];

			// Get the full path to the module's file.
			if (GetModuleFileNameEx(hProcess, hMods[i], szModName,
				sizeof(szModName) / sizeof(TCHAR)))
			{
				// Print the module name and handle value.
				KN_TRACE(LogSysMain, "  %s (0x%llX)", szModName, (uintptr_t)hMods[i]);
			}
		}
	}

	CloseHandle( hProcess );
	return true;
}

#endif /* _WIN32 */
