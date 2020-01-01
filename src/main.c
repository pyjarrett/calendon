#include "kn.h"

#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>

SDL_Window* window;
bool running = true;

void initAllSystems();
void initSDL();

/**
 * Create the window for drawing according to the available program
 * configuration.
 */
void createSDLWindow(const int width, const int height)
{
	const int windowInitFlags = SDL_WINDOW_OPENGL;
	window = SDL_CreateWindow("Powerblocks (knell)", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, width, height, windowInitFlags);
	if (window == NULL) {
		SDL_Quit();
		KN_FATAL_ERROR("Unable to create SDL windows\n");
	}
}

void initSDL()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		KN_FATAL_ERROR("Unable to init SDL");
	}

	const uint32_t width = 1024;
	const uint32_t height = 768;
	createSDLWindow(width, height);
}


/**
 * Initialize all systems.
 */
void initAllSystems(const int argc, char** argv)
{
	initSDL();
}

/**
 * Parses events off of the SDL event queue.
 */
void parseSDLEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
				//m_playerInput = m_playerInput.keyDown(event.key.keysym.sym);
				break;
			case SDL_KEYUP:
				//m_playerInput = m_playerInput.keyUp(event.key.keysym.sym);
				break;
			default:
				break;
		}
	}
}

int main(const int argc, char* argv[])
{
	initAllSystems(argc, argv);

	uint64_t last = timeNowNs();
	while (running) {
		//
		// Event checking should be quick.  Always processing events prevents
		// slowness due to bursts.
		//
		parseSDLEvents();

		//
		// Prevent updating too rapidly.  Maintaining a relatively consistent
		// timestep limits stored state and prevents precision errors due to
		// extremely small dt.
		//
		const uint64_t minTickSize = timeAsMs(8);
		const uint64_t current = timeNowNs();
		if (last > current) {
			KN_FATAL_ERROR("Time went backwards");
		}
		const uint64_t dt = current - last;
		if (dt < minTickSize) {
			continue;
		}

		last = current;

		//
		// Ignore obscenely large ticks, such as when resuming in the
		// debugger.
		//
		const uint64_t maxTickSize = timeAsSec(5);
		if (dt > maxTickSize) {
			continue;
		}

		// gGame.update(dt, m_playerInput);
		// gGraphics.startFrame();
		// gGame.draw();
		// gGraphics.endFrame();
	}
	return 0;
}
