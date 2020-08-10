#include "system.h"

CnCommandLineOptionList cnSystem_NoOptions(void)
{
	CnCommandLineOptionList options;
	options.options = NULL;
	options.numOptions = 0;
	return options;
}

void cnSystem_NoDefaultConfig(void* config)
{
	CN_UNUSED(config);
}

void* cnSystem_NoConfig(void)
{
	return NULL;
}


