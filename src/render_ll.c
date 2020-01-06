#include "render_ll.h"

#include "kn.h"
#include "color.h"
#include "log.h"

#include <stdbool.h>
#include <GL/gl.h>
#include <spa_fu/spa_fu.h>

extern struct SDL_Window* window;
static SDL_GLContext* gl;

static GLuint spriteProgram;

void rll_initGL()
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
		KN_FATAL_ERROR("Unable to create OpenGL context: %s", SDL_GetError());
	}
}

void rll_configureVSync()
{
	// VSync causes the draw to stall until the frame is displayed.
	// This might be more useful if/when drawing gets moved to it's own thread.
	const bool useVSync = true;
	if (useVSync) {
		SDL_GL_SetSwapInterval(1);
	}
	else {
		SDL_GL_SetSwapInterval(0);
	}
}

void rll_loadShaders()
{
	if (SPA_IsDir("assets")) {
		printf("assets dir exists\n");
	}
	else {
		printf("assets dir doesn't exist\n");
	}

	if (SPA_IsFile("assets")) {
		printf("assets is a file\n");
	}
	else {
		printf("assets is not a file\n");
	}
}


void rll_init()
{
	rll_initGL();
	rll_configureVSync();
	rll_loadShaders();
}

void rll_startFrame()
{
	SDL_GL_MakeCurrent(window, gl);
}

void rll_endFrame()
{
	SDL_GL_SwapWindow(window);
}

void rll_clear(rgba8i color)
{
	glClearColor(color.red, color.green, color.blue, color.alpha);
	glClear(GL_COLOR_BUFFER_BIT);
}

