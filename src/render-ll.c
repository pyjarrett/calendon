#include "render-ll.h"

#include "kn.h"

#include "assets.h"
#include "color.h"
#include "fileio.h"
#include "log.h"
#include "math.h"

#include <stdbool.h>
#include <GL/gl.h>
#include <spa_fu/spa_fu.h>


extern struct SDL_Window* window;
static SDL_GLContext* gl;

static GLuint spriteProgram;

static float4x4 projection;

float4x4 RLL_OrthoProjection(const uint32_t width, const uint32_t height)
{
	const float far = 0;
	const float near = 100;
	const float4x4 scale = float4x4_nonuniformScale(2.0f / width, 2.0f / height, 2.0f / (far - near));
	const float4x4 trans = float4x4_translate(-width / 2.0f, -height / 2.0f, -(far + near) / 2.0f);
	return float4x4_multiply(trans, scale);
}

void RLL_InitGL()
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

void RLL_ConfigureVSync()
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

void RLL_LoadShaders()
{
	const int pathMax = 1024;
	char path[pathMax];
	if (Assets_PathFor("shaders/screen_coord.frag", path, pathMax)) {
		if (SPA_IsFile(path)) {
			KN_TRACE(LogSysMain, "\e[32m" "%s found" "\e[39m", path);
		}
		else {
			KN_TRACE(LogSysMain, "%s not found", path);
		}
	}

	DynamicBuffer sourceFileBuffer;
	if (!File_Read(path, KN_FILE_TYPE_TEXT, &sourceFileBuffer)) {
		KN_ERROR(LogSysMain, "Unable to read path");
	}
	Mem_Free(&sourceFileBuffer);
}

void RLL_Init(const uint32_t width, const uint32_t height)
{
	RLL_InitGL();
	RLL_ConfigureVSync();
	RLL_LoadShaders();

	projection = RLL_OrthoProjection(width, height);
}

void RLL_StartFrame()
{
	SDL_GL_MakeCurrent(window, gl);
}

void RLL_EndFrame()
{
	SDL_GL_SwapWindow(window);
}

void RLL_Clear(rgba8i color)
{
	glClearColor(color.red, color.green, color.blue, color.alpha);
	glClear(GL_COLOR_BUFFER_BIT);
}

