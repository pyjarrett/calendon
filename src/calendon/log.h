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
 * Systems registered with the log system will log only warnings and errors
 * by default.
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
 * Simplifies the question of "What type is a log handle?"  The convention for
 * log handles is `LogSys<MySystem>` so the logging macros look like they're
 * using a type, not a variable.
 */
typedef uint32_t CnLogHandle;

/**
 * Global main program log.
 *
 * This is provided for quick convenience while troubleshooting issues quickly,
 * and is not intended for most messages in the long-term.  Registering systems
 * allows for log filters to quickly remove chaff from the salient parts.
 */
extern CN_API CnLogHandle LogSysMain;

/**
 * Names of all systems registered for use.  These names get added to every log
 * line to indicate the originating system of each message.
 */
extern CN_API const char* g_logSystemNames[CN_LOG_MAX_SYSTEMS];

/**
 * Prints a message to the given log handle.  Do not call this function directly,
 * instead use `CN_LOG`.
 */
CN_API void cnLog_Print(CnLogHandle system, CnLogVerbosity verbosity, const char* msg, ...);

/**
 * Records a message to the log system.
 */
#define CN_LOG(system, verbosity, msg, ...) \
	cnLog_Print(system, verbosity, \
		"%c: %40s:%i SYS_%-10s: " msg " \n", \
		g_logVerbosityChar[verbosity], \
		__FILE__, __LINE__, \
		g_logSystemNames[system], \
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

void            cnLog_PreInit(void);

CN_API bool     cnLog_IsReady(void);
CN_API uint32_t cnLog_RegisterSystem(const char* name);
CN_API void     cnLog_SetVerbosity(CnLogHandle system, uint32_t verbosity);
CN_API void     cnLog_SetEnabled(bool enabled);

CN_API void     cnLog_Shutdown(void);

CN_HEADER_END

#endif /* CN_LOG_H */
