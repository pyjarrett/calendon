/*
 * Goals
 * - draw a textured sprite
 *		- load an image into the renderer
 *		- draw a textured triangle strip with the image
 *
 * Out of scope
 * - tints
 * - texture operations (scaling, bordering, masking, etc.)
 */
#include <knell/kn.h>
#include <knell/assets.h>
#include <knell/log.h>
#include <knell/render.h>
#include <knell/path.h>

LogHandle LogSysSample;

static SpriteId spriteId;

KN_GAME_API bool Game_Init(void)
{
	Log_RegisterSystem(&LogSysSample, "Sample", KN_LOG_TRACE);
	KN_TRACE(LogSysSample, "Sample loaded");

	// Get the path to load.
	PathBuffer spritePath;
	Assets_PathBufferFor("sprites/stick_person.png", &spritePath);

	// Load the data into a temporary format in memory.

	// Create a handle for the sprite to go to.
	if (!R_CreateSprite(&spriteId)) {
		KN_FATAL_ERROR("Unable to create sprite");
	}

	// Create a texture from the temporary format and set up the texture
	// environment.
	if (!R_LoadSprite(spriteId, spritePath.str)) {
		KN_FATAL_ERROR("Unable to load texture for sprite: '%s'", spritePath.str);
	}
	return true;
}

KN_GAME_API void Game_Draw(void)
{
	R_StartFrame();

	float2 position = float2_Make(200, 200);
	Dimension2f	dimensions = { .width = 100, .height = 200 };

	R_DrawSprite(spriteId, position, dimensions);

	R_EndFrame();
}

KN_GAME_API void Game_Tick(uint64_t dt)
{
}

KN_GAME_API void Game_Shutdown(void)
{
}
