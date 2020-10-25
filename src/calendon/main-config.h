#ifndef CN_MAIN_CONFIG_H
#define CN_MAIN_CONFIG_H

#include <calendon/command-line-option.h>
#include <calendon/path.h>
#include <calendon/behavior.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	CnBehavior payload;
	CnPathBuffer gameLibPath;
	int64_t tickLimit;
	bool headless;
} CnMainConfig;

void* cnMain_Config(void);
void cnMain_SetDefaultConfig(void* config);
CnCommandLineOptionList cnMain_CommandLineOptionList(void);

#ifdef __cplusplus
}
#endif

#endif /* CN_MAIN_CONFIG_H */
