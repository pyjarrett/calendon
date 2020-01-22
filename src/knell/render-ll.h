/*
 * Low-level render control.
 *
 * The low level renderer performs the draw calls and resource management which
 * allow drawing for the game.
 */
#ifndef KN_RENDER_LL_H
#define KN_RENDER_LL_H

#include "color.h"
#include "math2.h"
#include "math4.h"

#ifdef _WIN32
	#include "compat-windows.h"

	#include <GL/glew.h>
	#define GL_GLEXT_PROTOTYPES 1
	#include <GL/gl.h>
	#include <SDL_opengl_glext.h>
#else // linux
	// Get prototypes without manually loading each one.
	#define GL_GLEXT_PROTOTYPES 1
	#include <GL/gl.h>
	#include <GL/glext.h>
#endif

#if KN_DEBUG
	#define KN_ASSERT_NO_GL_ERROR() RLL_CheckGLError(__FILE__, __LINE__)
	const char* RLL_GLTypeToString(GLenum type);
	void RLL_PrintProgram(GLuint program);
	void RLL_PrintGLVersion();
	void RLL_CheckGLError(const char* file, int line);
#else
	#define KN_ASSERT_NO_GL_ERROR()
#endif

void RLL_Init(uint32_t width, uint32_t height);
void RLL_StartFrame(void);
void RLL_EndFrame(void);
void RLL_Clear(rgba8i color);

void RLL_DrawDebugFullScreenRect();
void RLL_DrawDebugRect(float4 position, dimension2f dimensions, float4 color);
void RLL_DrawDebugLine(float x1, float y1, float x2, float y2, rgb8 color);
void RLL_DrawDebugLineStrip(float2* points, uint32_t numPoints, rgb8 color);

#endif /* KN_RENDER_LL_H */

