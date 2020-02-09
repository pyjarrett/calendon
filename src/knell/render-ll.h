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
#include "sprite.h"

void RLL_Init(uint32_t width, uint32_t height);
void RLL_StartFrame(void);
void RLL_EndFrame(void);
void RLL_Clear(rgba8i color);

bool RLL_CreateSprite(SpriteId* id);
bool RLL_LoadSprite(SpriteId id, const char* path);
void RLL_DrawSprite(SpriteId id, float2 position, dimension2f size);

void RLL_DrawDebugFullScreenRect(void);
void RLL_DrawDebugRect(float4 center, dimension2f dimensions, float4 color);
void RLL_DrawDebugLine(float x1, float y1, float x2, float y2, rgb8 color);
void RLL_DrawDebugLineStrip(float2* points, uint32_t numPoints, rgb8 color);

#endif /* KN_RENDER_LL_H */

