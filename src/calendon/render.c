#include "render.h"

#include "math4.h"
#include "render-ll.h"

/**
 * Initialize the rendering system assuming a rectangular region of the given
 * drawing dimensions.
 */
void cnR_Init(CnDimension2u32 resolution)
{
	cnRLL_Init(resolution);
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

	cnRLL_SetViewport(cnR_BackingCanvasAABB2());

	const CnRGBA8u black = { 0, 0, 0, 0 };
	cnRLL_Clear(black);
}

/**
 * The frame is now done and should be submitted for drawing.
 */
void cnR_EndFrame(void)
{
	cnRLL_EndFrame();
}

CnDimension2u32 cnR_Resolution(void)
{
	return cnRLL_Resolution();
}

CnAABB2 cnR_BackingCanvasAABB2(void)
{
	return cnRLL_BackingCanvasArea();
}

CnAABB2 cnR_Viewport(void)
{
	return cnRLL_Viewport();
}

/**
 * Sets the area for drawing on the screen which acts like a "viewport" into the
 * world being drawn.  The viewport must be given in units of the underlying
 * resolution.
 *
 * The viewport must be contained entirely within the backing canvas area,
 * otherwise you'd be drawing off the screen which is likely an error.
 *
 * @see cnR_BackingCanvasAABB2
 */
void cnR_SetViewport(CnAABB2 viewport)
{
	CN_ASSERT(cnAABB2_FullyContainsAABB2(cnR_BackingCanvasAABB2(), viewport, 0.0f),
		"Viewport is not fully contained by the backing canvas.");
	cnRLL_SetViewport(viewport);
}

/**
 * Sets the camera to draw a specified area of the coordinate system being
 * rendered.  The center of the area will be at the center of the drawn
 * viewport.
 *
 * Differences between the aspect ratio of the area being drawn and the viewport
 * will result in distortion (squishing or stretching) or the image.
 */
void cnR_SetCameraAABB2(CnAABB2 area)
{
	cnRLL_SetCameraAABB2(area);
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
	return cnRLL_CreateFont(id);
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
	params.layout = CnLayoutDirectionHorizontal;
	params.printDirection = CnTextDirectionLeftToRight;
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

void cnR_DrawRect(CnFloat2 center, CnDimension2f dimensions, CnRGB8u color, CnTransform2 transform)
{
	cnRLL_DrawRect(center, dimensions,
		cnFloat4_Make((float) color.r / 255.0f, (float) color.g / 255.0f, (float) color.b / 255.0f, 1.0f),
		cnRLL_MatrixFromTransform(transform));
}

void cnR_OutlineRect(CnFloat2 center, CnDimension2f dimensions, CnRGB8u color, CnTransform2 transform)
{
	cnRLL_OutlineRect(center, dimensions,
		cnFloat4_Make((float) color.r / 255.0f, (float) color.g / 255.0f, (float) color.b / 255.0f, 1.0f),
		cnRLL_MatrixFromTransform(transform));
}

void cnR_OutlineCircle(CnFloat2 center, float radius, CnRGB8u color, uint32_t numSegments)
{
	cnRLL_OutlineCircle(center, radius, color, numSegments);
}
