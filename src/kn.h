/*
 * Functions and types common to many parts of Knell.
 */
#include <stdbool.h>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>

/**
 * An unrecoverable event happened at this point in the program.
 */
#define KN_FATAL_ERROR(error_message, ...) \
	printf(error_message, ##__VA_ARGS__); \
	abort();

/**
 * Used to suppress errors resulting from unused values.
 */
#define KN_UNUSED(value) (void)(value)

#if defined(_DEBUG) || !defined(NDEBUG)
	#define KN_DEBUG 1
#else
	#define KN_DEBUG 0
#endif

#if KN_DEBUG
	#if defined(_MSC_VER)
		#if DY_CONFIG_DEBUG
			#include <intrin.h>
			#define KN_DEBUG_BREAK() __debugbreak();
		#endif
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

/*
 * Functions for getting the current time and converting other units to the
 * same units being used for time.
 */
uint64_t timeNowNs();
uint64_t msToNs(uint64_t ms);
uint64_t secToNs(uint64_t sec);

