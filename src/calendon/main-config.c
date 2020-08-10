#include "main-config.h"

#include <calendon/cn.h>
#include <calendon/argparse.h>

#include <string.h>

bool cnMainConfig_IsHosted(CnMainConfig* config)
{
	CN_ASSERT(config, "Cannot determine if null config is hosted.");
	return cnPlugin_IsComplete(&config->payload);
}

void cnMainConfig_Freestanding(CnMainConfig* config, CnPlugin_InitFn init,
	CnPlugin_TickFn tick, CnPlugin_DrawFn draw, CnPlugin_ShutdownFn shutdown)
{
	CN_ASSERT(config, "Cannot create a freestanding implementation from a null config.");

	config->payload.init = init;
	config->payload.tick = tick;
	config->payload.draw = draw;
	config->payload.shutdown = shutdown;
	config->payload.sharedLibrary = NULL;

	if (!cnMainConfig_IsHosted(config)) {
		CN_FATAL_ERROR("Improperly hosted config.");
	}
}
