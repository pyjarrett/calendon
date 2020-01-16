#include "render.h"

#include "math.h"
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

	//RLL_DrawDebugFullScreenRect();

	float4 origin = float4_Make(100.0f, 100.0f, 0.0f, 1.0f);
	float4 white = float4_Make(1.0f, 1.0f, 1.0f, 1.0f);
	dimension2f sample = { 10.0f, 20.0f };
	RLL_DrawDebugRect(origin, sample, white);
}

void R_EndFrame()
{
	RLL_EndFrame();
}
