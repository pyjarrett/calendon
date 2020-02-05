#include "render.h"

#include "math4.h"
#include "render-ll.h"

/**
 * Initialize the rendering system assuming a rectangular region of the given
 * drawing dimensions.
 */
KN_API void R_Init(uint32_t width, uint32_t height)
{
	RLL_Init(width, height);
}

/**
 * To be called once per main loop to reset the state required to draw the next
 * frame.  Error conditions should be restored and any pending values should be
 * considered invalid.
 *
 * All calls to render operations should be between a `R_StartFrame()` and
 * `R_EndFrame()`.
 */
KN_API void R_StartFrame(void)
{
	RLL_StartFrame();

	const rgba8i black = { 0, 0, 0, 0 };
	RLL_Clear(black);
}

/**
 * The frame is now done and should be submitted for drawing.bookmarks
 */
KN_API void R_EndFrame(void)
{
	RLL_EndFrame();
}

KN_API bool R_CreateSprite(SpriteId* id)
{
	KN_ASSERT(id != NULL, "Cannot assign a sprite to a null pointer.");
	return RLL_CreateSprite(id);
}

KN_API bool R_LoadSprite(SpriteId id, const char* path)
{
	return RLL_LoadSprite(id, path);
}

KN_API void R_DrawSprite(SpriteId id, float2 position, dimension2f size)
{
	RLL_DrawSprite(id, position, size);
}

KN_API void R_DrawDebugFullScreenRect(void)
{
	RLL_DrawDebugFullScreenRect();
}

KN_API void R_DrawDebugRect(float2 position, dimension2f dimensions, rgb8 color)
{
	RLL_DrawDebugRect(float4_Make(position.x, position.y, 0.0f, 1.0f),
		dimensions, float4_Make((float)color.r / 255.0f,
			(float)color.g / 255.0f, (float)color.b / 255.0f, 1.0f));
}

KN_API void R_DrawDebugLine(float x1, float y1, float x2, float y2, rgb8 color)
{
	RLL_DrawDebugLine(x1, y1, x2, y2, color);
}

KN_API void R_DrawDebugLineStrip(float2* points, uint32_t numPoints, rgb8 color)
{
	RLL_DrawDebugLineStrip(points, numPoints, color);
}
