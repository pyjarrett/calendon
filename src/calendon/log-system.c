#include "log-system.h"

#include <calendon/cn.h>

#include <calendon/log.h>
#include <calendon/log-config.h>

static CnLogConfig s_config;

const char* cnLog_Name(void)
{
    return "Log";
}

void* cnLog_Config(void)
{
    return &s_config;
}

void cnLog_SetDefaultConfig(void* config)
{
    CnLogConfig* c = (CnLogConfig*)config;
    c->enabled = true;
}

bool cnLog_Init(void)
{
    cnLog_SetEnabled(s_config.enabled);
    return true;
}

CnSystem cnLog_System(void)
{
    return (CnSystem) {
            .name             = cnLog_Name,
            .options          = cnLog_CommandLineOptionList,
            .config           = cnLog_Config,
            .setDefaultConfig = cnLog_SetDefaultConfig,

            .init             = cnLog_Init,
            .shutdown         = cnLog_Shutdown,
            .sharedLibrary    = NULL,

            .behavior         = cnSystem_NoBehavior()
    };
}
