/*
 * Compatability header to simplify inclusion of SDL.
 */
#ifndef COMPAT_SDL_H
#define COMPAT_SDL_H

#ifdef _WIN32
	#include <SDL.h>
	#include <SDL_rect.h>
#else
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_rect.h>
#endif

#endif /* COMPAT_SDL_H */
