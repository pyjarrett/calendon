#include "render.h"

#include "math4.h"
#include "render-ll.h"

/**
 * Initialize the rendering system assuming a rectangular region of the given
 * drawing dimensions.
 */
void cnR_Init(uint32_t width, uint32_t height)
{
	cnRLL_Init(width, height);
}

void cnR_Shutdown(void)
{
	cnRLL_Shutdown();
}

/**
 * To be called once per main loop to reset the state required to draw the next
 * frame.  Error conditions should be restored and any pending values should be
 * considered invalid.
 *
 * All calls to render operations should be between a `cnR_StartFrame()` and
 * `cnR_EndFrame()`.
 */
void cnR_StartFrame(void)
{
	cnRLL_StartFrame();

	const CnRGBA8u black = { 0, 0, 0, 0 };
	cnRLL_Clear(black);
}

/**
 * The frame is now done and should be submitted for drawing.bookmarks
 */
void cnR_EndFrame(void)
{
	cnRLL_EndFrame();
}

bool cnR_CreateSprite(CnSpriteId* id)
{
	CN_ASSERT(id != NULL, "Cannot assign a sprite to a null pointer.");
	return cnRLL_CreateSprite(id);
}

bool cnR_LoadSprite(CnSpriteId id, const char* path)
{
	return cnRLL_LoadSprite(id, path);
}

void cnR_DrawSprite(CnSpriteId id, CnFloat2 position, CnDimension2f size)
{
	cnRLL_DrawSprite(id, position, size);
}

bool cnR_CreateFont(CnFontId* id)
{
	return RLL_CreateFont(id);
}

bool cnR_LoadPSF2Font(CnFontId id, const char* path)
{
	return cnRLL_LoadPSF2Font(id, path);
}

void cnR_DrawSimpleText(CnFontId id, CnFloat2 position, const char* text)
{
	CnTextDrawParams params;
	params.position = position;
	params.color = (CnRGBA8u) { .red = 255, .green = 255, .blue = 255, .alpha = 255 };
	params.layout = CnLayoutHorizontal;
	params.printDirection = CnPrintDirectionLeftToRight;
	cnRLL_DrawSimpleText(id, &params, text);
}

void cnR_DrawDebugFullScreenRect(void)
{
	cnRLL_DrawDebugFullScreenRect();
}

void cnR_DrawDebugRect(CnFloat2 center, CnDimension2f dimensions, CnRGB8u color)
{
	cnRLL_DrawDebugRect(center, dimensions, cnFloat4_Make((float) color.r / 255.0f,
		(float) color.g / 255.0f, (float) color.b / 255.0f, 1.0f));
}

void cnR_DrawDebugLine(float x1, float y1, float x2, float y2, CnRGB8u color)
{
	cnRLL_DrawDebugLine(x1, y1, x2, y2, color);
}

void cnR_DrawDebugLineStrip(CnFloat2* points, uint32_t numPoints, CnRGB8u color)
{
	cnRLL_DrawDebugLineStrip(points, numPoints, color);
}

void cnR_DrawDebugFont(CnFontId id, CnFloat2 center, CnDimension2f size)
{
	cnRLL_DrawDebugFont(id, center, size);
}

void cnR_OutlineCircle(CnFloat2 center, float radius, CnRGB8u color, uint32_t numSegments)
{
	cnRLL_OutlineCircle(center, radius, color, numSegments);
}
