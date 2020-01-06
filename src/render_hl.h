/*
 * High-level render control.
 *
 * High-level control is the actual goals to accomplish during rendering, such
 * as indicating frame start/end, clearing the screen, drawing a background, or
 * drawing a sprite at a specific location.  The details of what exactly is
 * happening is hidden from the game itself since the high-level control hides
 * the details of how these details occur.
 *
 * Any sort of rendering backend should be able to implement the functions
 * defined here and be able to render the scene appropriately.
 *
 * This engine supports 2D graphics only.  This includes sprites, lines,
 * text, and polygons.
 */
#ifndef KN_RENDER_HL_H
#define KN_RENDER_HL_H

#include "kn.h"
#include "color.h"

void rhl_init();
void rhl_startFrame();
void rhl_clear(rgba8i color);
void rhl_endFrame();

typedef uint32_t SpriteID;
SpriteID rhl_createSprite();
void rhl_drawSprite(SpriteID spriteID, float x, float y);

#endif /* KN_RENDER_HL_H */
