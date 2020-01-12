/*
 * Low-level render control.
 *
 * The low level renderer performs the draw calls and resource management which
 * allow drawing for the game.
 */
#ifndef KN_RENDER_LL_H
#define KN_RENDER_LL_H

#include "color.h"
#include "math.h"

// Get prototypes without manually loading each one.
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>

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
void RLL_StartFrame();
void RLL_EndFrame();
void RLL_Clear(rgba8i color);

void RLL_DrawDebugFullScreenRect();

#endif /* KN_RENDER_LL_H */

