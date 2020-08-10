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

#include <calendon/argparse.h>
#include <calendon/plugin.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	CnCommandLineOption* options;
	size_t numOptions;
} CnCommandLineOptionList;

typedef CnCommandLineOptionList (*CnSystem_CommandLineOptionsListFn)(void);
typedef CnPlugin (*CnSystem_PluginFn)(void);

/**
 * Returns the location of the system's configuration struct.
 */
typedef void* (*CnSystem_ConfigFn)(void);

/**
 * Sets the config to default values.
 */
typedef void (*CnSystem_SetDefaultConfigFn)(void*);

CnCommandLineOptionList cnSystem_NoOptions(void);
void cnSystem_NoDefaultConfig(void*);
void* cnSystem_NoConfig(void);

/**
 * A system which not only operates like a plugin, but which provides runtime
 * configuration.  System initialization order depends upon their the
 * topological sort of all of their dependencies.
 */
typedef struct {
	const char* name;
	CnSystem_CommandLineOptionsListFn options;
	CnSystem_PluginFn plugin;
	CnSystem_ConfigFn config;
	CnSystem_SetDefaultConfigFn setDefaultConfig;
} CnSystem;

/**
 * A function which describes a system.
 */
typedef CnSystem (*CnSystem_SystemFn)(void);

#ifdef __cplusplus
}
#endif

#endif /* CN_SYSTEM_H */
