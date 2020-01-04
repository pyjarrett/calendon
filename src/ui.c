#include "ui.h"

#include "control.h"
#include "input.h"

SDL_Window* window;
KeyInputs keyInputs;

/**
 * Create the window for drawing according to the available program
 * configuration.
 */
static void createWindow(const int width, const int height)
{
	const int windowInitFlags = SDL_WINDOW_OPENGL;
	window = SDL_CreateWindow("Powerblocks (knell)", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, width, height, windowInitFlags);
	if (window == NULL) {
		SDL_Quit();
		KN_FATAL_ERROR("Unable to create SDL windows\n");
	}
}


void initWindow()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		KN_FATAL_ERROR("Unable to init SDL");
	}

	const uint32_t width = 1024;
	const uint32_t height = 768;
	createWindow(width, height);
}

/**
 * Parses events off of the SDL event queue.
 */
void processWindowEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				queueGracefulShutdown();
				break;
			case SDL_KEYDOWN:
				KeySet_add(&keyInputs.down, event.key.keysym.sym);
				KeySet_remove(&keyInputs.up, event.key.keysym.sym);
				break;
			case SDL_KEYUP:
				KeySet_add(&keyInputs.up, event.key.keysym.sym);
				KeySet_remove(&keyInputs.down, event.key.keysym.sym);
				break;
			default:
				break;
		}
	}
}

