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

KN_GAME_API void Game_Init(void)
{
	Log_RegisterSystem(&LogSysSample, "Sample", KN_LOG_TRACE);
	KN_TRACE(LogSysSample, "Sample loaded");

	// Get the path to load.
	PathBuffer spritePath;
    memset(&spritePath, 0, sizeof(PathBuffer));
	Assets_PathFor("sprites/stick_person.png", spritePath.path, KN_PATH_MAX);
    spritePath.length = (uint32_t)strlen(spritePath.path);

	// Load the data into a temporary format in memory.

	// Create a handle for the sprite to go to.
	if (!R_CreateSprite(&spriteId)) {
		KN_FATAL_ERROR("Unable to create sprite");
	}

	// Create a texture from the temporary format and set up the texture
	// environment.
	if (!R_LoadSprite(spriteId, spritePath.path)) {
		KN_FATAL_ERROR("Unable to load texture for sprite: '%s'", spritePath.path);
	}
}

KN_GAME_API void Game_Draw(void)
{
	R_StartFrame();

	float2 position = float2_Make(200, 200);
	dimension2f	dimensions = { .width = 100, .height = 200 };

	R_DrawSprite(spriteId, position, dimensions);

	R_EndFrame();
}

KN_GAME_API void Game_Tick(uint64_t dt)
{
}

KN_GAME_API void Game_Shutdown()
{
}
