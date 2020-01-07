#include "render.h"

#include "render-ll.h"

static SpriteID nextSpriteID = 0;

void rhl_init()
{
	rll_init();
}

void rhl_startFrame()
{
	rll_startFrame();

	rgba8i black = { 0, 0, 0, 0 };
	rll_clear(black);
}

void rhl_endFrame()
{
	rll_endFrame();
}

SpriteID rhl_createSprite()
{
	return nextSpriteID++;
}

void rhl_drawSprite(SpriteID spriteID, float x, float y)
{
}

