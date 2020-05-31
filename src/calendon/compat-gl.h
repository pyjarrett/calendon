#ifndef CN_COMPAT_GL_H
#define CN_COMPAT_GL_H

/*
 * Compatibility header for bringing in OpenGL.
 */

#if defined(_WIN32)
	#include <calendon/compat-windows.h>
	#include <GL/glew.h>

// Bring in additional OpenGL function names.
	#define GL_GLEXT_PROTOTYPES 1
	#include <GL/gl.h>
	#include <SDL_opengl_glext.h>
#elif defined(__linux__)
// Get prototypes without manually loading each one.
	#define GL_GLEXT_PROTOTYPES 1
	#include <GL/gl.h>
	#include <GL/glext.h>
#else
	#error "OpenGL rendering is not supported on this platform."
#endif

#endif /* CN_COMPAT_GL_H */
