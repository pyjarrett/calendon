#ifndef CN_PLUGIN_H
#define CN_PLUGIN_H

/**
 * Interface to be implemented by plugins, games, and demos.
 *
 * Plugins start by having their `Plugin_Init` function called.  Every tick, both
 * `Plugin_Tick` and `Plugin_Draw` are called in that order.  Immediately before
 * the plugin is unloaded `Plugin_Shutdown` is called.
 *
 * `Plugin_Init` may indicate failure in the payload initializing by returning
 * false.  `CN_FATAL_ERROR` may also be used to indicate failures in Calendon
 * related components.
 */

#include <calendon/cn.h>

#include <calendon/shared-library.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*CnPlugin_InitFn)(void);
typedef void (*CnPlugin_DrawFn)(void);
typedef void (*CnPlugin_TickFn)(uint64_t);
typedef void (*CnPlugin_ShutdownFn)(void);

typedef struct {
	CnPlugin_InitFn init;
	CnPlugin_DrawFn draw;
	CnPlugin_TickFn tick;
	CnPlugin_ShutdownFn shutdown;

	/**
	 * The library from which this plugin was loaded.  If not loaded from a
	 * shared library, this might be NULL.
	 */
	cnSharedLibrary sharedLibrary;
} CnPlugin;

bool cnPlugin_LoadFromFile(CnPlugin* plugin, const char* sharedLibraryName);

#ifdef __cplusplus
}
#endif

#endif /* CN_PLUGIN_H */
