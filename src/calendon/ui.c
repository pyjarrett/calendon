#include "ui.h"

#include <calendon/control.h>

SDL_Window* window;
static uint32_t width, height;
CnInput lastInput;

/**
 * Create the window for drawing according to the available program
 * configuration.
 */
static void cnUI_CreateWindow(const uint32_t w, const uint32_t h)
{
	const uint32_t windowInitFlags = SDL_WINDOW_OPENGL;
	window = SDL_CreateWindow("Powerblocks (calendon)", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, (int)w, (int)h, windowInitFlags);
	if (window == NULL) {
		SDL_Quit();
		CN_FATAL_ERROR("Unable to create SDL windows\n");
	}
}

void cnUI_Init(uint32_t width, uint32_t height)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		CN_FATAL_ERROR("Unable to init SDL");
	}
	cnUI_CreateWindow(width, height);
	width = width;
	height = height;
}

void cnUI_Shutdown(void)
{
	if (window) {
		SDL_DestroyWindow(window);
	}
	SDL_Quit();
}

/**
 * Parses events off of the SDL event queue.
 */
void cnUI_ProcessWindowEvents(void)
{
	SDL_Event event;
	bool mouseMoved = false;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				cnMain_QueueGracefulShutdown();
				break;
			case SDL_KEYDOWN:
				cnKeySet_Add(&lastInput.keySet.down, event.key.keysym.sym);
				cnKeySet_Remove(&lastInput.keySet.up, event.key.keysym.sym);
				break;
			case SDL_KEYUP:
				cnKeySet_Add(&lastInput.keySet.up, event.key.keysym.sym);
				cnKeySet_Remove(&lastInput.keySet.down, event.key.keysym.sym);
				break;
			case SDL_MOUSEMOTION:
				// SDL mouse motion is recorded in accordance with an origin in
				// the top left, so convert to a cartesian coordiante system for
				// inputs.
				mouseMoved = true;
				cnMouse_Move(&lastInput.mouse, event.motion.x, (int32_t) height - event.motion.y, event.motion.xrel,
							 -event.motion.yrel);
				break;
			default:
				break;
		}
	}

	if (!mouseMoved) {
		cnMouse_Still(&lastInput.mouse);
	}
}

CnInput* cnUI_InputPoll(void)
{
	// TODO: Not the preferred the way to do this since it doesn't indicate
	// how long the pointer lasts;
	return &lastInput;
}
