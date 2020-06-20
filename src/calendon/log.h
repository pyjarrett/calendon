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
#ifndef CN_LOG_H
#define CN_LOG_H

#include "cn.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Maximum number of systems supported by the logging system.
 */
#define CN_LOG_MAX_SYSTEMS 64

/**
 * Simplifies the question of "What type is a log handle?"
 */
typedef uint32_t CnLogHandle;

/**
 * Global main program log.
 */
extern CN_API CnLogHandle LogSysMain;

/**
 * Names of all systems registered for use.  These names get added to every log
 * line to indicate the originating system of each message.
 */
extern CN_API const char* LogSystemsRegistered[CN_LOG_MAX_SYSTEMS];

/**
 * Log verbosity settings in increasing order of logging.  Using a higher
 * setting enables logging from all systems below it.
 */
typedef enum {
	// Since unsigned ints are used, GCC generates lots of compiler warnings
	// relating to (verbosity <= 0) since verbosity is unsigned and cannot be
	// less than 0.  This "always on" error mode hides that error.
	CnLogVerbosityFatal = 0,

	/** Very serious issues requiring attention. */
	CnLogVerbosityError = 1,

	/**
	 * Warnings to be fixed before shipping.  The program can continue
	 * executing, but it is in an abnormal or reduced state.
	 */
	CnLogVerbosityWarn = 2,

	/** Messages for tracing program flow. */
	CnLogVerbosityTrace = 3,

	/** Total number of verbosity settings. */
	CnLogVerbosityNum
} CnLogVerbosity;

/**
 * Characters printed in the line to represent the type of log entry.
 */
extern CN_API char LogVerbosityChar[CnLogVerbosityNum];

/**
 * Per-system verbosity settings given by values in `CN_LOG_*`.  Every system
 * has its own verbosity settings so you can disable the ones for spammy
 * systems while you ridicule their authors.
 */
extern CN_API uint32_t LogSystemsVerbosity[CN_LOG_MAX_SYSTEMS];

/**
 * Records a message to the log system.
 */
#define CN_LOG(system, verbosity, msg, ...) do { \
	if (verbosity <= LogSystemsVerbosity[system]) { \
		printf("%c: %40s:%i SYS_%-10s: " msg " \n", \
			LogVerbosityChar[verbosity], \
			__FILE__, \
			__LINE__, \
			LogSystemsRegistered[system], \
			##__VA_ARGS__ \
			); \
    } } while(0)

/**
 * Errors are serious issues which must be addressed prior to shipping a product
 * and fixed as soon as possible when detected in release.
 */
#define CN_ERROR(system, msg, ...) \
	CN_LOG(system, CnLogVerbosityError, msg, ##__VA_ARGS__); \
	CN_DEBUG_BREAK()

/**
 * A program should never give warnings in a regular run of the program.
 */
#define CN_WARN(system, msg, ...) CN_LOG(system, CnLogVerbosityWarn, msg, ##__VA_ARGS__)

/**
 * Trace is a level used for debugging.
 */
#define CN_TRACE(system, msg, ...) CN_LOG(system, CnLogVerbosityTrace, msg, ##__VA_ARGS__)

CN_API bool cnLog_IsReady(void);
CN_API void cnLog_Init(void);
CN_API void cnLog_Shutdown(void);
CN_API void cnLog_RegisterSystem(CnLogHandle* system, const char* name, uint32_t verbosity);

#ifdef __cplusplus
}
#endif

#endif /* CN_LOG_H */