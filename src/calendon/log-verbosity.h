#ifndef CN_LOG_VERBOSITY_H
#define CN_LOG_VERBOSITY_H

#include <calendon/cn.h>

CN_HEADER_BEGIN_EXPORTED

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

CN_HEADER_END

#endif /* CN_LOG_VERBOSITY_H */
