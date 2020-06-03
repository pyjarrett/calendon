/*
 * Compatability header to simplify inclusion of SDL.
 */
#ifndef CN_COMPAT_SDL_H
#define CN_COMPAT_SDL_H

#ifdef _WIN32
	#include <SDL.h>
	#include <SDL_rect.h>
#else
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_rect.h>
#endif

#endif /* CN_COMPAT_SDL_H */
