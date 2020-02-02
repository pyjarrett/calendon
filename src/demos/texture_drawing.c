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

LogHandle LogSysSample;

/**
 * Maximum path length varies by system, but assume a reasonable case.
 */
#define KN_PATH_MAX 256

/*
 * Verify the path length assumption against the OS.
 */
#ifdef __linux__
	#include <linux/limits.h>
	KN_STATIC_ASSERT(KN_PATH_MAX <= PATH_MAX,
		"Knell allows larger path sizes than the OS");
#endif

/**
 * Rather than proliferate a ridiculous number of buffers and sizes all over the
 * place, agree on a "reasonable" path length.  This has the side-benefit of
 * allowing for joins to occur without additional allocations, and allowing
 * for quick buffer creation on the stack.
 */
typedef struct {
	char path[KN_PATH_MAX];
	uint32_t length;
} PathBuffer;

KN_STATIC_ASSERT(KN_PATH_MAX <= sizeof(PathBuffer),
	"PathBuffer is not big enough");

static SpriteId spriteId;

KN_GAME_API void Game_Init(void)
{
	Log_RegisterSystem(&LogSysSample, "Sample", KN_LOG_TRACE);
	KN_TRACE(LogSysSample, "Sample loaded");

	// Get the path to load.
	PathBuffer spritePath;
    memset(&spritePath, 0, sizeof(PathBuffer));
	Assets_PathFor("sprites/test_sprite.png", spritePath.path, KN_PATH_MAX);
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
