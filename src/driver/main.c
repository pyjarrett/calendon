/**
 * @file main.c
 *
 * The main driver for the engine.  This driver loads the game library (.so or
 * .dll) at runtime, allowing game and demo writers to focus on creating their
 * demo, and not on redoing boilerplate.  In addition to saving time, this
 * drastically reduces the complexity and amount of demo code, and compilation
 * time.
 *
 * Since knell is a single-threaded engine, the driver starts up and shuts down
 * the core game systems, runs the main loop, calculates the time delta since
 * the last frame and executes the update and draw code provided by the game
 * library.
 *
 * The game uses `Main_*` functions in `control_*` to ask the driver to
 * shutdown.
 */
#include <knell/driver.h>

int main(int argc, char* argv[])
{
	knDriverConfig config;
	if (!knDriver_ParseCommandLine(argc, argv, &config)) {
		return EXIT_FAILURE;
	}

	knDriver_Init(&config);
	knDriver_MainLoop();
	knDriver_Shutdown();
	return EXIT_SUCCESS;
}
