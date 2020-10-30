#pragma once

#include <calendon/cn.h>

#include <calendon/command-line-option.h>
#include <calendon/log-settings.h>

CN_HEADER_BEGIN_PRIVATE

typedef struct {
	bool enabled;
} CnLogConfig;

CnCommandLineOptionList cnLog_CommandLineOptionList(void);
void*                   cnLog_Config(void);
void                    cnLog_SetDefaultConfig(void*);

CN_HEADER_END
