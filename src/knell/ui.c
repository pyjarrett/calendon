#include "ui.h"

#include "control.h"
#include "input.h"

SDL_Window* window;
KeyInputs keyInputs;

/**
 * Create the window for drawing according to the available program
 * configuration.
 */
static void UI_CreateWindow(const uint32_t width, const uint32_t height)
{
	const uint32_t windowInitFlags = SDL_WINDOW_OPENGL;
	window = SDL_CreateWindow("Powerblocks (knell)", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, (int)width, (int)height, windowInitFlags);
	if (window == NULL) {
		SDL_Quit();
		KN_FATAL_ERROR("Unable to create SDL windows\n");
	}
}

KN_API void UI_Init(uint32_t width, uint32_t height)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		KN_FATAL_ERROR("Unable to init SDL");
	}
	UI_CreateWindow(width, height);
}

KN_API void UI_Shutdown(void)
{
	if (window) {
		SDL_DestroyWindow(window);
	}
	SDL_Quit();
}

/**
 * Parses events off of the SDL event queue.
 */
KN_API void UI_ProcessWindowEvents(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				Main_QueueGracefulShutdown();
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
