#include "render.h"

#include "math.h"
#include "render-ll.h"

static float4x4 projection;

float4x4 orthoProjection(const uint32_t width, const uint32_t height)
{
	const float far = 0;
	const float near = 100;
	const float4x4 scale = float4x4_nonuniformScale(2.0f / width, 2.0f / height, 2.0f / (far - near));
	const float4x4 trans = float4x4_translate(-width / 2.0f, -height / 2.0f, -(far + near) / 2.0f);
	return float4x4_multiply(trans, scale);
}

void R_Init(const uint32_t width, const uint32_t height)
{
	projection = orthoProjection(width, height);
	RLL_Init();
}

void R_StartFrame()
{
	RLL_StartFrame();

	rgba8i black = { 0, 0, 0, 0 };
	RLL_Clear(black);
}

void R_EndFrame()
{
	RLL_EndFrame();
}
