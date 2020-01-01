#include "kn.h"
#include "input.h"
#include "render_hl.h"

#include <stdio.h>

SDL_Window* window;

static bool running = true;
static KeyInputs keyInputs;

static void initAllSystems();
static void initSDL();

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

void initSDL()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		KN_FATAL_ERROR("Unable to init SDL");
	}

	const uint32_t width = 1024;
	const uint32_t height = 768;
	createWindow(width, height);
}

/**
 * Initialize all systems.
 */
void initAllSystems(const int argc, char** argv)
{
	initSDL();
	rhl_init();
}

/**
 * Parses events off of the SDL event queue.
 */
void parseSDLEvents(KeyInputs* keyInputs)
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
				KeySet_add(&keyInputs->down, event.key.keysym.sym);
				KeySet_remove(&keyInputs->up, event.key.keysym.sym);
				break;
			case SDL_KEYUP:
				KeySet_add(&keyInputs->up, event.key.keysym.sym);
				KeySet_remove(&keyInputs->down, event.key.keysym.sym);
				break;
			default:
				break;
		}
	}
}

int main(const int argc, char* argv[])
{
	initAllSystems(argc, argv);

	printf("Sizeof KeyInputs: %i", sizeof(KeyInputs));

	uint64_t last = timeNowNs();
	while (running) {
		//
		// Event checking should be quick.  Always processing events prevents
		// slowness due to bursts.
		//
		parseSDLEvents(&keyInputs);

		if (keyInputs.down.size > 0 || keyInputs.up.size > 0) {
			printf("Keys down: %i\n", keyInputs.down.size);
			printf("Keys up: %i\n", keyInputs.up.size);
		}

		//
		// Prevent updating too rapidly.  Maintaining a relatively consistent
		// timestep limits stored state and prevents precision errors due to
		// extremely small dt.
		//
		const uint64_t minTickSize = msToNs(8);
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
		const uint64_t maxTickSize = secToNs(5);
		if (dt > maxTickSize) {
			printf("Skipping large tick");
			continue;
		}

		rgba8i black = { 0, 0, 0, 0 };
		rhl_startFrame();
		rhl_clear(black);
		rhl_endFrame();

		// gGame.update(dt, m_playerInput);
		// gGraphics.startFrame();
		// gGame.draw();
		// gGraphics.endFrame();
	}
	return 0;
}
