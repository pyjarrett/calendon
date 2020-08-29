#ifndef CN_MAIN_CONFIG_H
#define CN_MAIN_CONFIG_H

#include <calendon/command-line-option.h>
#include <calendon/path.h>
#include <calendon/plugin.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	CnPlugin payload;
	CnPathBuffer gameLibPath;
	int64_t tickLimit;
	bool headless;
} CnMainConfig;

void* cnMain_Config(void);
void cnMain_SetDefaultConfig(void* config);
CnCommandLineOptionList cnMain_CommandLineOptionList(void);

CN_API void cnMainConfig_Freestanding(CnPlugin_InitFn init, CnPlugin_TickFn tick, CnPlugin_DrawFn draw, CnPlugin_ShutdownFn shutdown);

#ifdef __cplusplus
}
#endif

#endif /* CN_MAIN_CONFIG_H */
