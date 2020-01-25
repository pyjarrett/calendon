#include "ui.h"

#include "control.h"
#include "input.h"

SDL_Window* window;
static uint32_t width, height;
Input lastInput;

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

KN_API void UI_Init(uint32_t w, uint32_t h)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		KN_FATAL_ERROR("Unable to init SDL");
	}
	UI_CreateWindow(w, h);
	width = w;
	height = h;
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
	bool mouseMoved = false;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				Main_QueueGracefulShutdown();
				break;
			case SDL_KEYDOWN:
				KeySet_Add(&lastInput.keySet.down, event.key.keysym.sym);
				KeySet_Remove(&lastInput.keySet.up, event.key.keysym.sym);
				break;
			case SDL_KEYUP:
				KeySet_Add(&lastInput.keySet.up, event.key.keysym.sym);
				KeySet_Remove(&lastInput.keySet.down, event.key.keysym.sym);
				break;
			case SDL_MOUSEMOTION:
				// SDL mouse motion is recorded in accordance with an origin in
				// the top left, so convert to a cartesian coordiante system for
				// inputs.
				mouseMoved = true;
				Mouse_Move(&lastInput.mouse, event.motion.x, (int32_t)height - event.motion.y, event.motion.xrel, -event.motion.yrel);
				break;
			default:
				break;
		}
	}

	if (!mouseMoved) {
		Mouse_Still(&lastInput.mouse);
	}
}

KN_API Input* UI_InputPoll(void)
{
	// TODO: Not the preferred the way to do this since it doesn't indicate
	// how long the pointer lasts;
	return &lastInput;
}
