#ifndef CN_RENDER_LL_H
#define CN_RENDER_LL_H

/**
 * @file render-ll.h
 *
 * Low-level render control.
 *
 * Any sort of rendering backend should be able to implement the functions
 * defined here and be able to render the scene appropriately.
 *
 * The low level renderer performs the draw calls and resource management which
 * allow drawing for the game.
 */

#include <calendon/color.h>
#include <calendon/handle.h>
#include <calendon/math2.h>
#include <calendon/math4.h>
#include <calendon/render-resources.h>

void cnRLL_Init(CnDimension2u32 resolution);
void cnRLL_Shutdown(void);
void cnRLL_StartFrame(void);
void cnRLL_EndFrame(void);
void cnRLL_Clear(CnRGBA8u color);

CnDimension2u32 cnRLL_Resolution(void);

CnAABB2 cnRLL_BackingCanvasArea(void);

CnAABB2 cnRLL_Viewport(void);
void cnRLL_SetViewport(CnAABB2 viewport);

CnAABB2 cnRLL_CameraAABB2(void);
void cnRLL_SetCameraAABB2(const CnAABB2 mapSlice);

CN_DEFINE_HANDLE_TYPE(CnSpriteId, cnRLL_, Sprite);
CN_DEFINE_HANDLE_TYPE(CnFontId, cnRLL_, Font);

CnFloat4x4 cnRLL_MatrixFromTransform(CnTransform2 transform);

bool cnRLL_LoadSprite(CnSpriteId id, const char* path);
void cnRLL_DrawSprite(CnSpriteId id, CnFloat2 position, CnDimension2f size);

bool cnRLL_LoadPSF2Font(CnFontId id, const char* path);
void cnRLL_DrawSimpleText(CnFontId id, CnTextDrawParams* params, const char* text);
void cnRLL_DrawDebugFont(CnFontId id, CnFloat2 center, CnDimension2f size);

void cnRLL_DrawDebugFullScreenRect(void);
void cnRLL_DrawDebugRect(CnFloat2 center, CnDimension2f dimensions, CnOpaqueColor color);
void cnRLL_DrawDebugLine(float x1, float y1, float x2, float y2, CnOpaqueColor color);
void cnRLL_DrawDebugLineStrip(CnFloat2* points, uint32_t numPoints, CnOpaqueColor color);

void cnRLL_DrawRect(CnFloat2 center, CnDimension2f dimensions, CnOpaqueColor color, CnFloat4x4 transform);
void cnRLL_OutlineRect(CnFloat2 center, CnDimension2f dimensions, CnOpaqueColor color, CnFloat4x4 transform);

void cnRLL_OutlineCircle(CnFloat2 center, float radius, CnOpaqueColor color, uint32_t numSegments);

void cnRLL_FillScreen(CnOpaqueColor color);

#endif /* CN_RENDER_LL_H */
