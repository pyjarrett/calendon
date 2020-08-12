#include "cn.h"

#include <stdarg.h>
#include <stdio.h>

CN_API char fatalErrorBuffer[fatalErrorBufferLength];

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
	abort();
	va_end(args);
}
#else
#define CN_FATAL_ERROR(error_message, ...) \
	do { \
		printf(error_message, ##__VA_ARGS__); \
		fflush(stdout); \
		CN_DEBUG_BREAK(); \
		abort(); \
	} while (0)
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
