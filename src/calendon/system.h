#ifndef CN_SYSTEM_H
#define CN_SYSTEM_H

/**
 * @file system.h
 *
 * The basic building block of behavior in Calendon is the "system".
 *
 * A game is an assemblage of systems.
 *
 * Systems provide configuration, initialization and shutdown, and main loop
 * update behavior in a single unit.  All of these three elements are optional.
 *
 * Systems may have behavior which executes during the 4 phase tick of the
 * game update loop.  Some systems, like core's "Memory" system provide support
 * for other systems, but have no behavior of their own.
 *
 */
#include <calendon/cn.h>

#include <calendon/command-line-option.h>
#include <calendon/behavior.h>
#include <calendon/shared-library.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Systems must provide allocation for their own name to prevent complexity
 * from memory allocations issues dealing with dynamically loaded systems.
 */
typedef const char* (*CnSystem_NameFn)(void);

/**
 * Initialization can fail.
 */
typedef bool (*CnSystem_InitFn)(void);
typedef void (*CnSystem_ShutdownFn)(void);

typedef CnCommandLineOptionList (*CnSystem_CommandLineOptionsListFn)(void);

/**
 * Returns the location of the system's configuration struct.
 */
typedef void* (*CnSystem_ConfigFn)(void);

/**
 * Sets the config to default values.
 */
typedef void (*CnSystem_SetDefaultConfigFn)(void*);

/**
 * Provide a command line option list which has no options.
 */
CnCommandLineOptionList cnSystem_NoOptions(void);

/**
 * A noop function to do nothing to set a default config.
 */
void cnSystem_NoDefaultConfig(void*);

/**
 * Returns a NULL pointer to a config, indicating that the system has no
 * storage for a config struct.
 */
void* cnSystem_NoConfig(void);

/**
 * An empty behavior which does nothing.
 */
CnBehavior cnSystem_NoBehavior(void);

typedef struct {
	/**
	 * The name of the system which will be used as the prefix for the various
	 * common functions to load.
	 */
	CnSystem_NameFn name;

	// Configuration control.
	CnSystem_CommandLineOptionsListFn options;
	CnSystem_ConfigFn config;
	CnSystem_SetDefaultConfigFn setDefaultConfig;

	// Lifecycle control.
	CnSystem_InitFn init;
	CnSystem_ShutdownFn shutdown;

	// Behaviors to be used by the system.
	CnBehavior behavior;

	/**
	 * The library from which this plugin was loaded.  If not loaded from a
	 * shared library, this might be NULL.
	 */
	CnSharedLibrary sharedLibrary;
} CnSystem;

/**
 * A function returning a description of a system.
 */
typedef CnSystem (*CnSystem_SystemFn)(void);

bool cnSystem_LoadFromSharedLibrary(CnSystem* system, const char* name, CnSharedLibrary library);

#ifdef __cplusplus
}
#endif

#endif /* CN_SYSTEM_H */
