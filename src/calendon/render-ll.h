/*
 * Low-level render control.
 *
 * The low level renderer performs the draw calls and resource management which
 * allow drawing for the game.
 */
#ifndef CN_RENDER_LL_H
#define CN_RENDER_LL_H

#include <calendon/color.h>
#include <calendon/handle.h>
#include <calendon/math2.h>
#include <calendon/math4.h>
#include <calendon/render-resources.h>

void cnRLL_Init(uint32_t width, uint32_t height);
void cnRLL_Shutdown(void);
void cnRLL_StartFrame(void);
void cnRLL_EndFrame(void);
void cnRLL_Clear(CnRGBA8u color);

CN_DEFINE_HANDLE_TYPE(CnSpriteId, cnRLL_, Sprite);
CN_DEFINE_HANDLE_TYPE(CnFontId, cnRLL_, Font);

bool cnRLL_LoadSprite(CnSpriteId id, const char* path);
void cnRLL_DrawSprite(CnSpriteId id, CnFloat2 position, CnDimension2f size);

bool cnRLL_LoadPSF2Font(CnFontId id, const char* path);
void cnRLL_DrawSimpleText(CnFontId id, CnTextDrawParams* params, const char* text);
void cnRLL_DrawDebugFont(CnFontId id, CnFloat2 center, CnDimension2f size);

void cnRLL_DrawDebugFullScreenRect(void);
void cnRLL_DrawDebugRect(CnFloat2 center, CnDimension2f dimensions, CnFloat4 color);
void cnRLL_DrawDebugLine(float x1, float y1, float x2, float y2, CnRGB8u color);
void cnRLL_DrawDebugLineStrip(CnFloat2* points, uint32_t numPoints, CnRGB8u color);

void cnRLL_OutlineCircle(CnFloat2 center, float radius, CnRGB8u color, uint32_t numSegments);

#endif /* CN_RENDER_LL_H */

