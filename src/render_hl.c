#include "render_hl.h"

#include <GL/gl.h>

extern struct SDL_Window* window;
static SDL_GLContext* gl;

void rhl_init()
{
	// Get up and running quickly with OpenGL 3.1 with old-school functions.
	// TODO: Replace with Core profile once something is working.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	gl = SDL_GL_CreateContext(window);
	if (gl == NULL) {
		KN_FATAL_ERROR("Unable to create OpenGL context");
		//DY_FATAL_ERROR("Unable to create OpenGL context: %s", SDL_GetError());
	}

	// VSync causes the draw to stall until the frame is displayed.
	// This might be more useful if/when drawing gets moved to it's own thread.
	const bool useVSync = false;
	if (useVSync) {
		SDL_GL_SetSwapInterval(1);
	}
	else {
		SDL_GL_SetSwapInterval(0);
	}
}

void rhl_startFrame()
{
	SDL_GL_MakeCurrent(window, gl);
}

void rhl_clear(rgba8i color)
{
	glClearColor(color.red, color.green, color.blue, color.alpha);
	glClear(GL_COLOR_BUFFER_BIT);
}

void rhl_endFrame()
{
	SDL_GL_SwapWindow(window);
}

