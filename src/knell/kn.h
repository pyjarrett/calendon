/*
 * Functions and types common to many parts of Knell.
 *
 * The size of this prelude-style header should be reduced to minimum practical.
 */
#ifndef KN_H
#define KN_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Markers for engine library functions.
 *
 * - `KN_API` marker for engine library functions
 * - `KN_GAME_API` marker for game library functions
 *
 * Windows:
 * dllimport - describe that we're goign to find this when we link
 * dllexport - we're providing this function for others to use
 *
 * Linux:
 *  Overrides the hidden visibilty set by default by our build.
 * __attribute__((visibility("default"))
 *
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

#ifndef KN_DEBUG_BREAK
	#define KN_DEBUG_BREAK() do {} while(0)
#endif

/**
 * Runtime assert mechanism.
*/
#define KN_ASSERT(condition, message, ...) do { \
		if (!(condition)) { \
			KN_DEBUG_BREAK(); \
			KN_FATAL_ERROR(message, ##__VA_ARGS__); \
		} \
    } while (0)

/**
 * An unrecoverable event happened at this point in the program.
 */
#define KN_FATAL_ERROR(error_message, ...) \
	do { printf(error_message, ##__VA_ARGS__); abort(); } while (0)

/**
 * Used to suppress errors resulting from unused values.
 */
#define KN_UNUSED(value) (void)(value)

#endif /* KN_H */
