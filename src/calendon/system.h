#ifndef CN_SYSTEM_H
#define CN_SYSTEM_H

/**
 * @file system.h
 *
 * Systems link configuration capability with multiple plugins.  They are
 * describe initialization dependency order.
 *
 * A game is an assemblage of systems.  Not all systems exist at the same time,
 * and some provide supporting capabilities for others.
 *
 * As currently defined and implemented:
 * - "Plugin" : the grouping of initialization, shutdown, tick and draw
 * functions.  This is driven by the program driver itself.
 *
 * Improving definitions:
 * - "System" : A set of attributes and code to provide a capability and may
 *   have its own initialization and shutdown code.  Knowing that something is
 *   a system does not mean you know any of its actual capabilities.
 * - "Runtime System" : A system which provides a function to update its state
 *   for a discrete passage of time, called a "tick".
 * - "Lifetime System" : A system which exists throughout the life of the entire
 *   program, from program initialization to program shutdown.
 * - "Core System" : A member of a group of "lifetime system"s which get
 *   initialized prior to any other systems to provide basic functionality to
 *   the game.
 *     - Crash dumps    (Crash)
 *     - Logging        (Log)
 *     - Memory         (Mem)
 *     - Time           (Time)
 *     - Configuration  (Config)
 *     - User Interface (UI)
 *     - Rendering      (R)
 *
 * Program Lifecycle
 * 1. Program Initialization
 *   - The driver determines the core systems.
 *   - TODO: The driver reads core system configuration and issues configuration
 *     commands to the core systems.
 *   - Command line arguments get parsed to apply overrides to startup system
 *     configs.
 *
 * Z. Program Shutdown
 *
 * Extensions:
 * - Allowing systems to disable or enable other systems.
 *
 */
#include <calendon/cn.h>

#include <calendon/command-line-option.h>
#include <calendon/behavior.h>
#include <calendon/shared-library.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef CnCommandLineOptionList (*CnSystem_CommandLineOptionsListFn)(void);

typedef const char* (*CnSystem_NameFn)(void);

/**
 * Initialization can fail.
 */
typedef bool (*CnSystem_InitFn)(void);
typedef void (*CnSystem_ShutdownFn)(void);

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

/**
 * A system which not only operates like a plugin, but which provides runtime
 * configuration.
 *
 * System initialization order depends upon their the
 * topological sort of all of their dependencies.
 *
 * @todo provide descriptions of system dependencies
 * @todo sort system dependencies using a topological sort
 */
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
 * A function which describes a system.
 */
typedef CnSystem (*CnSystem_SystemFn)(void);

bool cnSystem_LoadFromSharedLibrary(CnSystem* system, const char* name, CnSharedLibrary library);

#ifdef __cplusplus
}
#endif

#endif /* CN_SYSTEM_H */
