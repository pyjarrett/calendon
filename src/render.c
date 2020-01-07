#include "render.h"

#include "math.h"
#include "render-ll.h"

static SpriteID nextSpriteID = 0;
static float4x4 projection;

float4x4 orthoProjection(const uint32_t width, const uint32_t height)
{
	const float far = 0;
	const float near = 100;
	const float4x4 scale = float4x4_nonuniformScale(2.0f / width, 2.0f / height, 2.0f / (far - near));
	const float4x4 trans = float4x4_translate(-width / 2.0f, -height / 2.0f, -(far + near) / 2.0f);
	return float4x4_multiply(trans, scale);
}

void r_init(const uint32_t width, const uint32_t height)
{
	projection = orthoProjection(width, height);
	rll_init();
}

void r_startFrame()
{
	rll_startFrame();

	rgba8i black = { 0, 0, 0, 0 };
	rll_clear(black);
}

void r_endFrame()
{
	rll_endFrame();
}

SpriteID r_createSprite()
{
	return nextSpriteID++;
}

void r_drawSprite(SpriteID spriteID, float x, float y)
{
}

