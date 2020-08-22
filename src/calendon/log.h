#ifndef CN_LOG_H
#define CN_LOG_H

/**
 * Logging system.
 *
 * I'm a little hesitant to add any sophisticated logging system because it
 * often gets more abuse than actual use.  The benefits of allowing program
 * tracing and reporting of errors without resorting to the debugger outweigh
 * the negative sides.
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
*/

#include <calendon/cn.h>

#include <calendon/log-settings.h>
#include <calendon/system.h>

CN_HEADER_BEGIN_EXPORTED

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

typedef struct {
	uint64_t counts[CnLogVerbosityNum];
} CnLogMessageCounter;

/**
 * Count the number of log messages produced by each system.
 */
extern CN_API CnLogMessageCounter LogMessagesProduced[CN_LOG_MAX_SYSTEMS];

/**
 * Characters printed in the line to represent the type of log entry.
 */
extern CN_API char LogVerbosityChar[CnLogVerbosityNum];

/**
 * Long descriptions of verbosity types.
 */
extern CN_API const char* LogVerbosityString[CnLogVerbosityNum];

/**
 * Per-system verbosity settings given by values in `CN_LOG_*`.  Every system
 * has its own verbosity settings so you can disable the ones for spammy
 * systems while you ridicule their authors.
 */
extern CN_API uint32_t LogSystemsVerbosity[CN_LOG_MAX_SYSTEMS];

CN_API void cnLog_Print(CnLogHandle system, CnLogVerbosity verbosity, const char* msg, ...);

/**
 * Records a message to the log system.
 */
#define CN_LOG(system, verbosity, msg, ...) \
	cnLog_Print(system, verbosity, \
		"%c: %40s:%i SYS_%-10s: " msg " \n", \
		LogVerbosityChar[verbosity], \
		__FILE__, __LINE__, \
		LogSystemsRegistered[system], \
		##__VA_ARGS__ \
		);

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

CnSystem cnLog_System(void);
CN_API bool cnLog_IsReady(void);
CN_API void cnLog_RegisterSystem(CnLogHandle* system, const char* name, uint32_t verbosity);

CN_HEADER_END

#endif /* CN_LOG_H */
