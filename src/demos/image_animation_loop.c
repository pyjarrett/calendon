/*
 * Working with animation loops.
 */
#include <knell/kn.h>
#include <knell/anim-loop.h>
#include <knell/assets.h>
#include <knell/log.h>
#include <knell/math2.h>
#include <knell/path.h>
#include <knell/render.h>
#include <knell/time.h>

LogHandle LogSysSample;

AnimationLoopCursor sampleCursor;
AnimationLoop sampleLoop;

#define SPRITE_ANIMATION_FRAMES 3
SpriteId spriteFrames[SPRITE_ANIMATION_FRAMES];

KN_GAME_API bool Plugin_Init(void)
{
	Log_RegisterSystem(&LogSysSample, "Sample", KN_LOG_TRACE);
	KN_TRACE(LogSysSample, "Sample loaded");

	KN_TRACE(LogSysSample, "Animation loop size:        %zu bytes", sizeof(AnimationLoop));
	KN_TRACE(LogSysSample, "Animation loop cursor size: %zu bytes", sizeof(AnimationLoopCursor));

	sampleLoop.numStates = SPRITE_ANIMATION_FRAMES;
	sampleLoop.elapsed[0] = Time_MsToNs(150);
	sampleLoop.elapsed[1] = Time_MsToNs(150);
	sampleLoop.elapsed[2] = Time_MsToNs(150);

	R_CreateSprite(&spriteFrames[0]);
	R_CreateSprite(&spriteFrames[1]);
	R_CreateSprite(&spriteFrames[2]);

	const char* frameFilenames[] = {
		"sprites/stick_person.png",
		"sprites/stick_person2.png",
		"sprites/stick_person3.png"
	};

	for (uint32_t i = 0; i < 3; ++i) {
		PathBuffer path;
		Assets_PathBufferFor(frameFilenames[i], &path);
		R_LoadSprite(spriteFrames[i], path.str);
	}
	return true;
}

KN_GAME_API void Plugin_Draw(void)
{
	R_StartFrame();

	float2 position = float2_Make(300, 300);
	Dimension2f size = { .width = 200.0f, .height = 200.0f };
	R_DrawSprite(spriteFrames[sampleCursor.current], position, size);

	R_EndFrame();
}

KN_GAME_API void Plugin_Tick(uint64_t dt)
{
	AnimLoop_Tick(&sampleLoop, &sampleCursor, dt);
}

KN_GAME_API void Plugin_Shutdown(void)
{
}
