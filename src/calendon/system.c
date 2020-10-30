#include "system.h"

#include <string.h>
#include <calendon/string.h>

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

/**
 * Many systems provide capabilities but do not need to tick every frame.
 */
CnBehavior cnSystem_NoBehavior(void)
{
	return (CnBehavior) {
		.beginFrame = NULL,
		.tick = NULL,
		.draw = NULL,
		.endFrame = NULL
	};
}

/**
 * Finds functions with the matching prefix, followed by _FunctionName for each
 * system function type: Name, Init, BeginFrame, Tick, Draw, and EndFrame.
 * e.g. "Physics" would find "Physics_Name", "Physics_Init", "Physics_Tick", etc.
 */
bool cnSystem_LoadFromSharedLibrary(CnSystem* system, const char* name, CnSharedLibrary library)
{
	CN_ASSERT_PTR(system);
	CN_ASSERT_PTR(name);
	CN_ASSERT_PTR(library);

	size_t systemNameLength;
	char functionName[256];
	memset(functionName, 0, 256);
	if (!cnString_NumCharacterBytes(name, 128, &systemNameLength)) {
		CN_FATAL_ERROR("System name is too long to be dynamically loaded: %s", name);
	}
	cnString_Copy(functionName, name, systemNameLength);

	char* functionNameStart = &functionName[0] + systemNameLength;

	system->sharedLibrary = library;

	system->options = cnSystem_NoOptions;
	system->config = cnSystem_NoConfig;
	system->setDefaultConfig = cnSystem_NoDefaultConfig;

	cnString_Format(functionNameStart, 256, "_Name");
	system->name = (CnSystem_NameFn) cnSharedLibrary_LookupFn(library, functionName);

	cnString_Format(functionNameStart, 256, "_Init");
	system->init = (CnSystem_InitFn) cnSharedLibrary_LookupFn(library, functionName);

	cnString_Format(functionNameStart, 256, "_BeginFrame");
	system->behavior.beginFrame = (CnBehavior_FrameFn) cnSharedLibrary_LookupFn(library, functionName);

	cnString_Format(functionNameStart, 256, "_Tick");
	system->behavior.tick = (CnBehavior_FrameFn) cnSharedLibrary_LookupFn(library, functionName);

	cnString_Format(functionNameStart, 256, "_Draw");
	system->behavior.draw = (CnBehavior_FrameFn) cnSharedLibrary_LookupFn(library, functionName);

	cnString_Format(functionNameStart, 256, "_EndFrame");
	system->behavior.endFrame = (CnBehavior_FrameFn) cnSharedLibrary_LookupFn(library, functionName);

	cnString_Format(functionNameStart, 256, "_Shutdown");
	system->shutdown = (CnSystem_ShutdownFn) cnSharedLibrary_LookupFn(library, functionName);

	return true;
}
