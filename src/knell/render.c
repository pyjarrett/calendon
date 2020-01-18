#include "render.h"

#include "math4.h"
#include "render-ll.h"

/**
 * Initialize the rendering system assuming a rectangular region of the given
 * drawing dimensions.
 */
KN_API void R_Init(const uint32_t width, const uint32_t height)
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
KN_API void R_StartFrame()
{
	RLL_StartFrame();

	const rgba8i black = { 0, 0, 0, 0 };
	RLL_Clear(black);

	float4 origin = float4_Make(100.0f, 100.0f, 0.0f, 1.0f);
	float4 white = float4_Make(1.0f, 1.0f, 1.0f, 1.0f);
	dimension2f sample = { 10.0f, 20.0f };
	RLL_DrawDebugRect(origin, sample, white);
}

/**
 * The frame is now done and should be submitted for drawing.
 */
KN_API void R_EndFrame()
{
	RLL_EndFrame();
}

KN_API void R_DrawDebugFullScreenRect(void)
{
	RLL_DrawDebugFullScreenRect();
}
