#include "cn.h"

#include <stdarg.h>
#include <stdio.h>

CN_API char fatalErrorBuffer[fatalErrorBufferLength];

#if defined(_WIN32) && defined(CN_DEBUG)
#include <calendon/compat-windows.h>
#include <memoryapi.h>
/**
 * EXPERIMENTAL
 *
 * A simple check to ensure access to the start of the pointer location.  This
 * does not guard against long ranges of access accesses, or verify that the
 * data at the pointer location is valid.
 *
 * This uses VirtualQuery instead of IsBadPtr or similar because I'm considering
 * using pointer checks to examine memory access patterns, such as to see
 * which data should be allocated together.
 *
 * https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualquery
 * https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-queryworkingset
 * https://stackoverflow.com/questions/8058005/mac-os-x-equivalent-of-virtualquery-or-proc-pid-maps
 * https://stackoverflow.com/questions/269314/is-there-a-better-way-than-parsing-proc-self-maps-to-figure-out-memory-protecti
 */
void cnValidatePtr(const void* ptr, const char* ptrName)
{
	MEMORY_BASIC_INFORMATION memInfo;
	const size_t bytesReturned = VirtualQuery(ptr, &memInfo, sizeof(memInfo));
	if (bytesReturned > 0) {
		const bool invalid = (memInfo.Protect & PAGE_NOACCESS) | (memInfo.Protect & PAGE_GUARD);
		CN_ASSERT(!invalid, "Invalid memory access: %s", ptrName);
	}
}
#endif

/*
 * An unrecoverable event happened at this point in the program.
 *
 * This causes a crash.  Use this when the program cannot recover from whatever
 * ill the program is in at this point.  Use `CN_FATAL_ERROR` to indicate
 * problems where the program was expected to succeed at an operation but
 * didn't, or an unrecoverable error occurred.
 */
#ifdef _WIN32
#include <calendon/compat-windows.h>
#include <debugapi.h>

void cnFatalError(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	if (IsDebuggerPresent()) {
		vprintf(msg, args);
		fflush(stdout);
		CN_DEBUG_BREAK();
	} else {
		vsnprintf(fatalErrorBuffer, fatalErrorBufferLength, msg, args);
		MessageBox(NULL, fatalErrorBuffer, "Fatal Error", MB_OK);
	}
	va_end(args);
	abort();
}
#else
void cnFatalError(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
	fflush(stdout);
	CN_DEBUG_BREAK();
	abort();
}
#endif

/**
 * Breaks the header dependency on stdio to minimize the size of cn.h.
 */
void cnPrint(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
}

CN_API int cnString_Format(char* str, size_t strLength, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	const int written = vsnprintf(str, strLength, format, args);
	va_end(args);
	return written;
}
