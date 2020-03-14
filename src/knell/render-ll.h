/*
 * Low-level render control.
 *
 * The low level renderer performs the draw calls and resource management which
 * allow drawing for the game.
 */
#ifndef KN_RENDER_LL_H
#define KN_RENDER_LL_H

#include <knell/color.h>
#include <knell/handle.h>
#include <knell/math2.h>
#include <knell/math4.h>
#include <knell/render-resources.h>

void RLL_Init(uint32_t width, uint32_t height);
void RLL_Shutdown(void);
void RLL_StartFrame(void);
void RLL_EndFrame(void);
void RLL_Clear(rgba8i color);

KN_DEFINE_HANDLE_TYPE(SpriteId, RLL_, Sprite);
KN_DEFINE_HANDLE_TYPE(FontId, RLL_, Font);

bool RLL_LoadSprite(SpriteId id, const char* path);
void RLL_DrawSprite(SpriteId id, float2 position, Dimension2f size);

bool RLL_LoadPSF2Font(FontId id, const char* path);
void RLL_DrawSimpleText(FontId id, TextDrawParams* params, const char* text);
void RLL_DrawDebugFont(FontId id, float2 center, Dimension2f size);

void RLL_DrawDebugFullScreenRect(void);
void RLL_DrawDebugRect(float2 center, Dimension2f dimensions, float4 color);
void RLL_DrawDebugLine(float x1, float y1, float x2, float y2, rgb8 color);
void RLL_DrawDebugLineStrip(float2* points, uint32_t numPoints, rgb8 color);

#endif /* KN_RENDER_LL_H */

