#pragma once

#include <calendon/cn.h>

#include <calendon/command-line-option.h>
#include <calendon/log-settings.h>
#include <calendon/memory.h>

CN_HEADER_BEGIN_PRIVATE

/**
 * Temporarily stores verbosity settings as read from the command line until log
 * initialization.
 */
typedef struct {
	CnDynamicBuffer systemNames;
	CnLogVerbosity verbositySettings[CN_LOG_MAX_SYSTEMS];
} CnLogPerSystemVerbositySettings;

typedef struct {
	bool enabled;
} CnLogConfig;

CnCommandLineOptionList cnLog_CommandLineOptionList(void);
void*                   cnLog_Config(void);
void                    cnLog_SetDefaultConfig(void*);

CN_HEADER_END
