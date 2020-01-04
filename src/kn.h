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

/*
 * Functions for getting the current time and converting other units to the
 * same units being used for time.
 */
uint64_t timeNowNs();
uint64_t msToNs(uint64_t ms);
uint64_t secToNs(uint64_t sec);

