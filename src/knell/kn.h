#ifndef KN_H
#define KN_H

/*
 * This is the "standard" Knell header which most files should include to get
 * the basic types and base functionality and macros used in Knell code.
 *
 * The size of this prelude-style header should be reduced to minimum practical
 * to prevent introducing excessive elements throughout the engine and bloating
 * compile times.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Symbol export/import markers for engine library functions.
 *
 * - Use `KN_API` for engine library functions (within knell-lib)
 * - Use `KN_GAME_API` for game library functions (in demo/game code)
 *
 * Windows:
 * - dllimport - describe that we're going to find this when we link
 * - dllexport - we're providing this function for others to use
 *
 * Linux:
 * - Overrides the hidden visibility set by default by our build.
 * - __attribute__((visibility("default")) the symbol should be made visible
 */
#ifdef _WIN32
	#if KN_LIBRARY
		#define KN_API __declspec(dllexport)
	#else
		#define KN_API __declspec(dllimport)
	#endif
	#define KN_GAME_API __declspec(dllexport)
#else
	#if KN_LIBRARY
		#define KN_API __attribute__((visibility("default")))
	#else
		#define KN_API __attribute__((visibility("default")))
	#endif
	#define KN_GAME_API __attribute((visibility("default")))
#endif /* WIN32 */

/*
 * Assert and other debug functionality should be able to trigger a breakpoint
 * in the debugger.
 *
 * This behavior is exposed globally to allow breakpoints on non-fatal
 * conditions to be placed in source control for difficult bugs which may
 * require multiple check-ins to diagnose and fix.
 *
 * Use cases:
 * - assertion failure
 * - excessive frame time
*/
#if KN_DEBUG
	#if defined(_MSC_VER)
		#include <intrin.h>
		#define KN_DEBUG_BREAK() __debugbreak();
	#endif

	#if defined(__GNUC__) || defined(__clang__)
		#include <signal.h>
		/**
		 * Use to force the debugger to stop at a specific line.
		 */
		#define KN_DEBUG_BREAK() raise(SIGTRAP)
	#endif
#endif

/*
 * KN_DEBUG_BREAK might be defined in release builds created with debug symbols
 * (e.g. RelWithDebugInfo).
 */
#ifndef KN_DEBUG_BREAK
	#define KN_DEBUG_BREAK() do {} while(0)
#endif

/**
 * Runtime assert mechanism.  `KN_ASSERT` is the preferred method of declaring
 * pre- and post-conditions within code, and also conditions which must be
 * true for correct code execution.
 *
 * `KN_ASSERT` is for declaring conditions which should only be violated as a
 * result of a bug, not an abnormal condition found during runtime (e.g. a
 * missing asset file).
*/
#define KN_ASSERT(condition, message, ...) do { \
		if (!(condition)) { \
			KN_FATAL_ERROR(message, ##__VA_ARGS__); \
		} \
    } while (0)

/*
 * Reserve space statically to write a fatal error message when things go wrong.
 */
enum { fatalErrorBufferLength = 1024 };
extern KN_API char fatalErrorBuffer[fatalErrorBufferLength];

/*
 * An unrecoverable event happened at this point in the program.
 *
 * This causes a crash.  Use this when the program cannot recover from whatever
 * ill the program is in at this point.  Use `KN_FATAL_ERROR` to indicate
 * problems where the program was expected to succeed at an operation but
 * didn't, or an unrecoverable error occurred.
 */
#ifdef _WIN32
	#include "compat-windows.h"
	#include <debugapi.h>
	#define KN_FATAL_ERROR(error_message, ...) \
		if (IsDebuggerPresent()) { \
			KN_DEBUG_BREAK(); \
            abort(); \
		} else { \
            snprintf(fatalErrorBuffer, fatalErrorBufferLength, "%s:%i\n" error_message, __FILE__, __LINE__, ##__VA_ARGS__); \
            MessageBox(NULL, fatalErrorBuffer, "Fatal Error", MB_OK); abort(); \
		}
#else
	#define KN_FATAL_ERROR(error_message, ...) \
		do { printf(error_message, ##__VA_ARGS__); abort(); } while (0)
#endif

/**
 * Used to suppress errors resulting from unused values.
 *
 * Typically this gets used during debugging to prevent from having to add and
 * remove function parameter names.  Long-term usage of `KN_UNUSED` is
 * discouraged and indicates that an API is likely to be deprecated or changed.
 */
#define KN_UNUSED(value) (void)(value)

/*
 * Macro to be used while writing code to indicate that this code should never
 * be submitted for real.  Define to something meaningless in production to
 * trigger a compilation error.
 */
#if KN_DEBUG
	#define KN_DO_NOT_SUBMIT
#else
	#define KN_DO_NOT_SUBMIT production_code_has_do_not_submit
#endif

#endif /* KN_H */
