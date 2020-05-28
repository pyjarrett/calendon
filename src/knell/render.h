/*
 * High-level render control.
 *
 * High-level control is the actual goals to accomplish during rendering, such
 * as indicating frame start/end, clearing the screen, drawing a background, or
 * drawing a sprite at a specific location.  The details of what exactly is
 * happening is hidden from the game itself since the high-level control hides
 * the details of how these details occur.  Rendering occurs only to a single
 * rectangular window.
 *
 * Any sort of rendering backend should be able to implement the functions
 * defined here and be able to render the scene appropriately.
 *
 * This engine supports 2D graphics only.  This includes sprites, lines,
 * text, and polygons.
 *
 * ## Design Notes
 *
 * There's a lot of duplication between this and the `render-ll.h`.  The
 * intention is to allow for cases where the interface provided to game clients
 * requires conversion to appropriate 3D functions to operate the back-end.
 */
#ifndef KN_RENDER_HL_H
#define KN_RENDER_HL_H

#include <knell/kn.h>

#include <knell/color.h>
#include <knell/math2.h>
#include <knell/render-resources.h>

#ifdef __cplusplus
extern "C" {
#endif

KN_API void R_Init(uint32_t width, uint32_t height);
KN_API void R_Shutdown(void);
KN_API void R_StartFrame(void);
KN_API void R_EndFrame(void);

KN_API bool R_CreateSprite(SpriteId* id);
KN_API bool R_LoadSprite(SpriteId id, const char* path);
KN_API void R_DrawSprite(SpriteId id, float2 position, Dimension2f size);

KN_API bool R_CreateFont(FontId* id);
KN_API bool R_LoadPSF2Font(FontId id, const char* path);
KN_API void R_DrawSimpleText(FontId id, float2 position, const char* text);

KN_API void R_DrawDebugFullScreenRect(void);
KN_API void R_DrawDebugRect(float2 center, Dimension2f dimensions, rgb8 color);
KN_API void R_DrawDebugLine(float x1, float y1, float x2, float y2, rgb8 color);
KN_API void R_DrawDebugLineStrip(float2* points, uint32_t numPoints, rgb8 color);
KN_API void R_DrawDebugFont(FontId id, float2 center, Dimension2f size);

KN_API void R_OutlineCircle(float2 center, float radius, rgb8 color, uint32_t numSegments);

#ifdef __cplusplus
}
#endif

#endif /* KN_RENDER_HL_H */
