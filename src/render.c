#include "render.h"

#include "render-ll.h"

void R_Init(const uint32_t width, const uint32_t height)
{
	RLL_Init(width, height);
}

void R_StartFrame()
{
	RLL_StartFrame();

	rgba8i black = { 0, 0, 0, 0 };
	RLL_Clear(black);

	RLL_DrawDebugFullScreenRect();
}

void R_EndFrame()
{
	RLL_EndFrame();
}
