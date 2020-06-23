/*
 * Working with animation loops.
 */
#include <calendon/cn.h>
#include <calendon/anim-loop.h>
#include <calendon/assets.h>
#include <calendon/input-button-mapping.h>
#include <calendon/input-digital-button.h>
#include <calendon/log.h>
#include <calendon/math2.h>
#include <calendon/path.h>
#include <calendon/render.h>
#include <calendon/time.h>
#include <calendon/ui.h>

CnLogHandle LogSysSample;

CnAnimationLoopCursor sampleCursor;
CnAnimationLoop sampleLoop;

#define SPRITE_ANIMATION_FRAMES 3
CnSpriteId spriteFrames[SPRITE_ANIMATION_FRAMES];

CnDigitalButton rightButton, leftButton;
CnButtonMapping buttonMapping;

CnFloat2 position;

CN_GAME_API bool CnPlugin_Init(void)
{
	cnLog_RegisterSystem(&LogSysSample, "Sample", CnLogVerbosityTrace);
	CN_TRACE(LogSysSample, "Sample loaded");

	CN_TRACE(LogSysSample, "Animation loop size:        %zu bytes", sizeof(CnAnimationLoop));
	CN_TRACE(LogSysSample, "Animation loop cursor size: %zu bytes", sizeof(CnAnimationLoopCursor));

	sampleLoop.numStates = SPRITE_ANIMATION_FRAMES;
	sampleLoop.elapsed[0] = cnTime_MakeMilli(150);
	sampleLoop.elapsed[1] = cnTime_MakeMilli(150);
	sampleLoop.elapsed[2] = cnTime_MakeMilli(150);

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

	cnButtonMapping_Map(&buttonMapping, SDLK_LEFT, &leftButton);
	cnButtonMapping_Map(&buttonMapping, SDLK_RIGHT, &rightButton);

	cnDigitalButton_Set(&rightButton, CnDigitalButtonStateUp);
	cnDigitalButton_Set(&leftButton, CnDigitalButtonStateUp);

	position = cnFloat2_Make(300, 300);

	return true;
}

CN_GAME_API void CnPlugin_Draw(void)
{
	cnR_StartFrame();

	CnDimension2f size = { .width = 200.0f, .height = 200.0f };
	cnR_DrawSprite(spriteFrames[sampleCursor.current], position, size);

	cnR_EndFrame();
}

void applyInputs(CnTime dt)
{
	cnInput_ApplyButtonMapping(cnInput_Poll(), &buttonMapping);
	const bool leftDown = cnDigitalButton_IsDown(&leftButton);
	const bool rightDown = cnDigitalButton_IsDown(&rightButton);
	const float moveRate = 0.30f;
	if (leftDown && !rightDown) {
		CnFloat2 left = cnFloat2_Make(-moveRate, 0.0f);
		position = cnFloat2_Add(position, cnFloat2_Multiply(left, (float)cnTime_Milli(dt)));
		cnAnimLoop_Tick(&sampleLoop, &sampleCursor, dt);
	} else if (rightDown && !leftDown) {
		CnFloat2 right = cnFloat2_Make(moveRate, 0.0f);
		position = cnFloat2_Add(position, cnFloat2_Multiply(right, (float)cnTime_Milli(dt)));
		cnAnimLoop_Tick(&sampleLoop, &sampleCursor, dt);
	}
}

CN_GAME_API void CnPlugin_Tick(CnTime dt)
{
	applyInputs(dt);
}

CN_GAME_API void CnPlugin_Shutdown(void)
{
}
