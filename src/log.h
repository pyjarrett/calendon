/*
 * Logging system.
 *
 * I'm a little hesitant to add any sophisticated logging system because it
 * often gets more abuse than actual use.  The benefits of allowing program
 * tracing and reporting of errors without resorting to the debugger outweigh
 * the negative sides.  I'm considering adding a portion of the system to
 * collect statistics of error and warning reports, but it is not sufficiently
 * important at this time.
 *
 * Logging systems help development by providing a running log of things which
 * have happened and variable inspection with minimal effect on program speed
 * when used judiciously.
 *
 * Goals of logging and assertion system design:
 * - simple to use
 * - minimal and lightweight
 * - requires no additional linking
 *
 * Purposes of logging:
 * - a timeline of major program events (system startup/shutdown)
 * - warnings and errors which occurred
 * - recording data when debugging
 *
 * Adding a logging entry should not be done on a whim, as adding entries
 * reduces program performance and makes logs more difficult to decipher.  Log
 * entries should also be easy to filter, and preferably not span multiple lines.
 *
 * Text log entries should provide:
 * 1) The system doing the logging.
 * 2) The severity of the log entry.
 * 3) The file and line of where the log entry is generated.
 * 4) The program time at which the log entry occurred.
 *
 * @subsection Reasons for not using `std::ostream` operators
 *
 * `std::ostream` locks users into writing against the `<<` operator. Also,
 * more in-depth formatting requires extensive documentation lookup and very
 * verbose code.
*/
#ifndef KN_LOG_H
#define KN_LOG_H

#include "kn.h"

#include <inttypes.h>
#include <stdint.h>

/**
 * Log verbosity settings in increasing order of logging.  Using a higher
 * setting enables logging from all systems below it.
 */
enum {
	/** Very serious issues requiring attention. */
	KN_LOG_ERROR = 0,

	/**
	 * Warnings to be fixed before shipping.  The program can continue
	 * executing, but it is in an abnormal or reduced state.
	 */
	KN_LOG_WARN,

	/** Messages for tracing program flow. */
	KN_LOG_TRACE,

	/** Total number of verbosity settings. */
	KN_LOG_NUM_TYPES
};

/**
 * Maximum number of systems supported by the logging system.
 */
#define KN_LOG_MAX_SYSTEMS 64

/**
 * Characters printed in the line to represent the type of log entry.
 */
extern char LogVerbosityChar[KN_LOG_NUM_TYPES];

/**
 * Per-system verbosity settings given by values in `KN_LOG_*`.  Every system
 * has its own verbosity settings so you can disable the ones for spammy
 * systems while you ridicule their authors.
 */
extern uint32_t LogSystemsVerbosity[KN_LOG_MAX_SYSTEMS];

/**
 * Names of all systems registered for use.  These names get added to every log
 * line to indicate the originating system of each message.
 */
extern const char* LogSystemsRegistered[KN_LOG_MAX_SYSTEMS];

/**
 * Records a message to the log system.
 */
#define KN_LOG(system, verbosity, msg, ...) \
		if (verbosity <= LogSystemsVerbosity[system]) { \
			printf("%c:" __FILE__ ":%i " "SYS_%s: " msg "\n", \
				LogVerbosityChar[verbosity], \
				__LINE__, \
				LogSystemsRegistered[system], ##__VA_ARGS__); \
		}

/**
 * Errors are serious issues which must be addressed prior to shipping a product
 * and fixed as soon as possible when detected in release.
 */
#define KN_ERROR(system, msg, ...) \
	KN_LOG(system, KN_LOG_ERROR, msg, ##__VA_ARGS__) \
	KN_DEBUG_BREAK();

/**
 * A program should never give warnings in a regular run of the program.
 */
#define KN_WARN(system, msg, ...) KN_LOG(system, KN_LOG_WARN, msg, ##__VA_ARGS__)

/**
 * Trace is a level used for debugging.
 */
#define KN_TRACE(system, msg, ...) KN_LOG(system, KN_LOG_TRACE, msg, ##__VA_ARGS__)

void LOG_Init();
void LOG_RegisterSystem(uint32_t* system, const char* name, uint32_t verbosity);

#endif /* KN_LOG_H */

