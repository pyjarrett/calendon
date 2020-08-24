#include "plugin.h"

/**
 * Loads a plugin's shared library and assigns its functions from a file.
 * Does not initialize the plugin.
 */
bool cnPlugin_LoadFromSharedLibrary(CnPlugin* plugin, CnSharedLibrary library)
{
	CN_ASSERT_NOT_NULL(plugin);
	CN_ASSERT_NOT_NULL(library);

	plugin->init = (CnPlugin_InitFn) cnSharedLibrary_LookupFn(library, "CnPlugin_Init");
	plugin->draw = (CnPlugin_DrawFn) cnSharedLibrary_LookupFn(library, "CnPlugin_Draw");
	plugin->tick = (CnPlugin_TickFn) cnSharedLibrary_LookupFn(library, "CnPlugin_Tick");
	plugin->shutdown = (CnPlugin_ShutdownFn) cnSharedLibrary_LookupFn(library, "CnPlugin_Shutdown");
	plugin->sharedLibrary = library;
	plugin->sharedLibrary = library;

	return true;
}

/**
 * A complete plugin has all plugin function pointers assigned.
 */
bool cnPlugin_IsComplete(CnPlugin* plugin)
{
	CN_ASSERT_NOT_NULL(plugin);
	return plugin->init
		&& plugin->shutdown
		&& plugin->draw
		&& plugin->tick;
}