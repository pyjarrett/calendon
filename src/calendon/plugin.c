#include "plugin.h"

#include <calendon/cn.h>

#include <calendon/shared-library.h>

/**
 * Loads a plugin's shared library and assigns its functions from a file.
 * Does not initialize the plugin.
 */
bool cnPlugin_LoadFromFile(CnPlugin* plugin, const char* sharedLibraryName)
{
	CN_ASSERT(plugin, "Cannot load to a null CnPlugin.");
	CN_ASSERT(sharedLibraryName, "Cannot load a CnPlugin from a null shared library name.");

	plugin->sharedLibrary = cnSharedLibrary_Load(sharedLibraryName);
	if (!plugin->sharedLibrary) {
		return false;
	}
	plugin->init = (CnPlugin_InitFn) cnSharedLibrary_LookupFn(plugin->sharedLibrary, "CnPlugin_Init");
	plugin->draw = (CnPlugin_DrawFn) cnSharedLibrary_LookupFn(plugin->sharedLibrary, "CnPlugin_Draw");
	plugin->tick = (CnPlugin_TickFn) cnSharedLibrary_LookupFn(plugin->sharedLibrary, "CnPlugin_Tick");
	plugin->shutdown = (CnPlugin_ShutdownFn) cnSharedLibrary_LookupFn(plugin->sharedLibrary, "CnPlugin_Shutdown");
	return true;
}
