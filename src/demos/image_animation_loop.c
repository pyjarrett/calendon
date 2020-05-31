/*
 * Working with animation loops.
 */
#include <calendon/cn.h>
#include <calendon/anim-loop.h>
#include <calendon/assets.h>
#include <calendon/log.h>
#include <calendon/math2.h>
#include <calendon/path.h>
#include <calendon/render.h>
#include <calendon/time.h>

CnLogHandle LogSysSample;

CnAnimationLoopCursor sampleCursor;
CnAnimationLoop sampleLoop;

#define SPRITE_ANIMATION_FRAMES 3
CnSpriteId spriteFrames[SPRITE_ANIMATION_FRAMES];

CN_GAME_API bool Plugin_Init(void)
{
	cnLog_RegisterSystem(&LogSysSample, "Sample", CN_LOG_TRACE);
	CN_TRACE(LogSysSample, "Sample loaded");

	CN_TRACE(LogSysSample, "Animation loop size:        %zu bytes", sizeof(CnAnimationLoop));
	CN_TRACE(LogSysSample, "Animation loop cursor size: %zu bytes", sizeof(CnAnimationLoopCursor));

	sampleLoop.numStates = SPRITE_ANIMATION_FRAMES;
	sampleLoop.elapsed[0] = cnTime_MsToNs(150);
	sampleLoop.elapsed[1] = cnTime_MsToNs(150);
	sampleLoop.elapsed[2] = cnTime_MsToNs(150);

	cnR_CreateSprite(&spriteFrames[0]);
	cnR_CreateSprite(&spriteFrames[1]);
	cnR_CreateSprite(&spriteFrames[2]);

	const char* frameFilenames[] = {
		"sprites/stick_person.png",
		"sprites/stick_person2.png",
		"sprites/stick_person3.png"
	};

	for (uint32_t i = 0; i < 3; ++i) {
		CnPathBuffer path;
		cnAssets_PathBufferFor(frameFilenames[i], &path);
		cnR_LoadSprite(spriteFrames[i], path.str);
	}
	return true;
}

CN_GAME_API void Plugin_Draw(void)
{
	cnR_StartFrame();

	CnFloat2 position = cnFloat2_Make(300, 300);
	CnDimension2f size = { .width = 200.0f, .height = 200.0f };
	cnR_DrawSprite(spriteFrames[sampleCursor.current], position, size);

	cnR_EndFrame();
}

CN_GAME_API void Plugin_Tick(uint64_t dt)
{
	cnAnimLoop_Tick(&sampleLoop, &sampleCursor, dt);
}

CN_GAME_API void Plugin_Shutdown(void)
{
}
