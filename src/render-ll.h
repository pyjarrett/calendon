/*
 * Low-level render control.
 *
 * The low level renderer performs the draw calls and resource management which
 * allow drawing for the game.
 */
#ifndef KN_RENDER_LL_H
#define KN_RENDER_LL_H

#include "color.h"

void RLL_Init(uint32_t width, uint32_t height);
void RLL_StartFrame();
void RLL_EndFrame();
void RLL_Clear(rgba8i color);

#endif /* KN_RENDER_LL_H */

