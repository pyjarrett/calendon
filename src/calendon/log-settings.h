#ifndef CN_LOG_SYSTEM_H
#define CN_LOG_SYSTEM_H

/**
 * @file log-settings.h
 *
 * Compile-time settings for the logging system.
 */
#include <calendon/cn.h>

CN_HEADER_BEGIN_EXPORTED

/**
 * Maximum number of systems supported by the logging system.
 */
#define CN_LOG_MAX_SYSTEMS 128

/**
 * Maximum length of name copied into the logging system.
 */
#define CN_LOG_MAX_SYSTEM_NAME_TERMINATED_LENGTH 32

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
extern CN_API char g_logVerbosityChar[CnLogVerbosityNum];

/**
 * Long descriptions of verbosity types.
 */
extern CN_API const char* g_logVerbosityString[CnLogVerbosityNum];

bool cnLog_IsValidVerbosity(uint32_t verbosity);

bool cnLog_VerbosityFromChar(char c, uint32_t* verbosity);

CN_HEADER_END

#endif /* CN_LOG_SYSTEM_H */
