#include "plugin.h"

#include <knell/kn.h>

#include <knell/shared-library.h>

/**
 * Loads a plugin's shared library and assigns its functions from a file.
 * Does not initialize the plugin.
 */
bool Plugin_LoadFromFile(Plugin* plugin, const char* sharedLibraryName)
{
	KN_ASSERT(plugin, "Cannot load to a null Plugin.");
	KN_ASSERT(sharedLibraryName, "Cannot load a Plugin from a null shared library name.");

	plugin->sharedLibrary = SharedLibrary_Load(sharedLibraryName);
	if (!plugin->sharedLibrary) {
		return false;
	}
	plugin->init = (Plugin_InitFn)SharedLibrary_LookupFn(plugin->sharedLibrary, "Plugin_Init");
	plugin->draw = (Plugin_DrawFn)SharedLibrary_LookupFn(plugin->sharedLibrary, "Plugin_Draw");
	plugin->tick = (Plugin_TickFn)SharedLibrary_LookupFn(plugin->sharedLibrary, "Plugin_Tick");
	plugin->shutdown = (Plugin_ShutdownFn)SharedLibrary_LookupFn(plugin->sharedLibrary, "Plugin_Shutdown");
	return true;
}
