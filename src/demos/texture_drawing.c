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
#include <calendon/cn.h>
#include <calendon/assets.h>
#include <calendon/log.h>
#include <calendon/render.h>
#include <calendon/path.h>
#include <calendon/time.h>

CnLogHandle LogSysSample;

static CnSpriteId spriteId;

CN_GAME_API bool CnPlugin_Init(void)
{
	cnLog_RegisterSystem(&LogSysSample, "Sample", CnLogVerbosityTrace);
	CN_TRACE(LogSysSample, "Sample loaded");

	// Get the path to load.
	CnPathBuffer spritePath;
	cnAssets_PathBufferFor("sprites/stick_person.png", &spritePath);

	// Load the data into a temporary format in memory.

	// Create a handle for the sprite to go to.
	if (!cnR_CreateSprite(&spriteId)) {
		CN_FATAL_ERROR("Unable to create sprite");
	}

	// Create a texture from the temporary format and set up the texture
	// environment.
	if (!cnR_LoadSprite(spriteId, spritePath.str)) {
		CN_FATAL_ERROR("Unable to load texture for sprite: '%s'", spritePath.str);
	}
	return true;
}

CN_GAME_API void CnPlugin_Draw(void)
{
	cnR_StartFrame();

	CnFloat2 position = cnFloat2_Make(200, 200);
	CnDimension2f	dimensions = { .width = 100, .height = 200 };

	cnR_DrawSprite(spriteId, position, dimensions);

	cnR_EndFrame();
}

CN_GAME_API void CnPlugin_Tick(CnTime dt)
{
}

CN_GAME_API void CnPlugin_Shutdown(void)
{
}
