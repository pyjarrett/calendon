#include "kn.h"
#include "input.h"
#include "render_hl.h"

#include <stdio.h>

SDL_Window* window;

static bool running = true;
static KeyInputs keyInputs;
static uint64_t lastTick;

static void initAllSystems();
static void initWindow();

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

void initAllSystems()
{
	lastTick = timeNowNs();
	initWindow();
	rhl_init();
}

/**
 * Parses events off of the SDL event queue.
 */
void parseSDLEvents(KeyInputs* inputs)
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
				KeySet_add(&inputs->down, event.key.keysym.sym);
				KeySet_remove(&inputs->up, event.key.keysym.sym);
				break;
			case SDL_KEYUP:
				KeySet_add(&inputs->up, event.key.keysym.sym);
				KeySet_remove(&inputs->down, event.key.keysym.sym);
				break;
			default:
				break;
		}
	}
}

void drawFrame()
{
	rgba8i black = {0, 0, 0, 0 };
	rhl_startFrame();
	rhl_clear(black);
	rhl_endFrame();
}

/**
 * Possibly generate a delta time for the next game update.  If the time since
 * the previous tick is too small or very large, no tick will be generated.
 * Small ticks do needless work, and large ticks might be due to resuming from
 * the debugger.
 *
 * @param[out] outDt delta time if a tick is generated (returns true), not set otherwise
 * @return true if a tick should occur
 */
bool generateTick(uint64_t* outDt)
{
	const uint64_t current = timeNowNs();
	if (lastTick > current) {
		KN_FATAL_ERROR("Time went backwards");
	}

	// Prevent updating too rapidly.  Maintaining a relatively consistent
	// timestep limits stored state and prevents precision errors due to
	// extremely small dt.
	//
	// Since Knell is single-threaded, VSync will probably ensure that the
	// minimum tick size is never missed.
	const uint64_t minTickSize = msToNs(8);
	const uint64_t dt = current - lastTick;
	if (dt < minTickSize) {
		return false;
	}

	lastTick = current;

	// Ignore huge ticks, such as when resuming in the debugger.
	const uint64_t maxTickSize = secToNs(5);
	if (dt > maxTickSize) {
		printf("Skipping large tick");
		return false;
	}

	*outDt = dt;
	return true;
}

void tick(uint64_t dt)
{
	KN_UNUSED(dt);
}

void runMainLoop()
{
	while (running) {
		// Event checking should be quick.  Always processing events prevents
		// slowness due to bursts.
		parseSDLEvents(&keyInputs);

		uint64_t dt;
		if (generateTick(&dt)) {
			tick(dt);
		}
		drawFrame();

		// gGame.update(dt, m_playerInput);
		// gGraphics.startFrame();
		// gGame.draw();
		// gGraphics.endFrame();
	}
}

int main(const int argc, char* argv[])
{
	initAllSystems();
	runMainLoop();
	return 0;
}
