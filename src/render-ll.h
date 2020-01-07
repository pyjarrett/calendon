/*
 * Low-level render control.
 *
 * The low level renderer performs the draw calls and resource management which
 * allow drawing for the game.
 */
#ifndef KN_RENDER_LL_H
#define KN_RENDER_LL_H

#include "color.h"

void rll_init();
void rll_startFrame();
void rll_endFrame();
void rll_clear(rgba8i color);

#endif /* KN_RENDER_LL_H */

