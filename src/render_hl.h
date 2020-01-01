/*
 * High-level render control.  Any sort of rendering backend should be able to
 * implement the functions defined here and be able to render the scene
 * appropriately.
 */
#ifndef KN_RENDER_HL_H
#define KN_RENDER_HL_H

#include "kn.h"

typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
} rgba8i;

void rhl_init();
void rhl_startFrame();
void rhl_clear(rgba8i color);
void rhl_endFrame();

#endif /* KN_RENDER_HL_H */

