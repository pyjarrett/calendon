#ifndef CN_DRIVER_CONFIG_H
#define CN_DRIVER_CONFIG_H

#include <calendon/path.h>
#include <calendon/plugin.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	CnPlugin payload;

	CnPathBuffer gameLibPath;

	int64_t tickLimit;
} CnMainConfig;

bool cnMainConfig_IsHosted(CnMainConfig* config);
CN_API bool cnMainConfig_ParseCommandLine(CnMainConfig* config, int argc, char** argv);
CN_API void cnMainConfig_Freestanding(CnMainConfig* config, CnPlugin_InitFn init, CnPlugin_TickFn tick, CnPlugin_DrawFn draw, CnPlugin_ShutdownFn shutdown);

#ifdef __cplusplus
}
#endif

#endif /* CN_DRIVER_CONFIG_H */
