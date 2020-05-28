#ifndef KN_PLUGIN_H
#define KN_PLUGIN_H

/**
 * Interface to be implemented by plugins, games, and demos.
 *
 * Plugins start by having their `Plugin_Init` function called.  Every tick, both
 * `Plugin_Tick` and `Plugin_Draw` are called in that order.  Immediately before
 * the plugin is unloaded `Plugin_Shutdown` is called.
 *
 * `Plugin_Init` may indicate failure in the payload initializing by returning
 * false.  `KN_FATAL_ERROR` may also be used to indicate failures in Knell
 * related components.
 */

#include <knell/kn.h>

#include <knell/shared-library.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*Plugin_InitFn)(void);
typedef void (*Plugin_DrawFn)(void);
typedef void (*Plugin_TickFn)(uint64_t);
typedef void (*Plugin_ShutdownFn)(void);

typedef struct {
	Plugin_InitFn init;
	Plugin_DrawFn draw;
	Plugin_TickFn tick;
	Plugin_ShutdownFn shutdown;

	/**
	 * The library from which this plugin was loaded.
	 */
	knSharedLibrary sharedLibrary;
} Plugin;

bool Plugin_LoadFromFile(Plugin* plugin, const char* sharedLibraryName);

#ifdef __cplusplus
}
#endif

#endif /* KN_PLUGIN_H */
