/*
 * Used to experiment with animation between discrete steps.
 */
#include <calendon/cn.h>
#include <calendon/input-keyset.h>
#include <calendon/log.h>
#include <calendon/math2.h>
#include <calendon/render.h>
#include <calendon/time.h>
#include <calendon/ui.h>

#include <math.h>

CnLogHandle LogSysSample;

/**
 * Stores the current state of an animation.
 *
 * This probably should be divided into multiple types.
 */
typedef struct {
	// Current position.
	CnFloat2 position;

	// Is this animation currently active?
	bool transitioning;

	// elapsed time, used to determine t, based on the animation rate
	CnTime elapsed;

	// The time it takes to change states.
	CnTime rate;

	// current interpolation value
	float t;

	// Current state of the animation.
	// need some sort of indication of which state being transitioned to.
	// TODO: Convert to a Transform2
	CnFloat2* last, *next;
} BinaryAnimation;

void Anim_Start(BinaryAnimation* anim)
{
	if (!anim->transitioning) {
		// Move to the next state transition.
		CnFloat2* temp = anim->last;
		anim->last = anim->next;
		anim->next = temp;

		anim->transitioning = true;
		anim->elapsed = cnTime_MakeZero();
		anim->rate = cnTime_MakeMilli(2000);
		anim->t = 0.0f;
		anim->position = cnFloat2_Add(cnFloat2_Multiply(*anim->last, 1.0f - anim->t),
									  cnFloat2_Multiply(*anim->next, anim->t));
	}
}

/**
 * Cause an existing animation in transition to reverse directions.
 */
void Anim_Reverse(BinaryAnimation* anim)
{
	if (!anim->transitioning) return;

	CnFloat2* temp = anim->last;
	anim->last = anim->next;
	anim->next = temp;

	anim->t = 1.0f - anim->t;
	anim->elapsed = cnTime_SubtractMonotonic(anim->rate, anim->elapsed);

	// Position should remain the same for this frame.
}

/**
 * Recalculates an animation position based upon its current "t" value.
 */
void Anim_Recalculate(BinaryAnimation *anim)
{
	anim->position = cnFloat2_Add(cnFloat2_Multiply(*anim->last, 1.0f - anim->t), cnFloat2_Multiply(*anim->next, anim->t));
}

void Anim_Complete(BinaryAnimation* anim)
{
	if (anim->transitioning && !cnTime_LessThan(anim->elapsed, anim->rate)) {
		anim->elapsed = anim->rate;
		anim->t = cnTime_Lerp(anim->elapsed, anim->rate);
		anim->transitioning = false;
	}
}

void Anim_Update(BinaryAnimation* anim, CnTime dt)
{
	if (anim->transitioning) {
		anim->elapsed = cnTime_Add(anim->elapsed, dt);
		anim->elapsed = cnTime_Min(anim->elapsed, anim->rate);
		anim->t = cnTime_Lerp(anim->elapsed, anim->rate);
		CN_ASSERT(0.0f <= anim->t && anim->t <= 1.0f, "Interpolation t is not in range [0, 1]");
		Anim_Recalculate(anim);
		Anim_Complete(anim);
	}
}

CnFloat2 left, right;
BinaryAnimation squareAnim;

CN_GAME_API bool CnPlugin_Init(void)
{
	LogSysSample = cnLog_RegisterSystem("Sample");
	cnLogHandle_SetVerbosity(LogSysSample, CnLogVerbosityTrace);
	CN_TRACE(LogSysSample, "Sample loaded");

	left = cnFloat2_Make(300, 300);
	right = cnFloat2_Make(500, 300);

	squareAnim.position = left;
	squareAnim.t = 0.0f;
	squareAnim.elapsed = cnTime_MakeZero();
	squareAnim.last = &right;
	squareAnim.next = &left;
	squareAnim.transitioning = false;
	return true;
}

CN_GAME_API void CnPlugin_Draw(void)
{
	cnR_StartFrame();

	const CnDimension2f rectSize = { 50, 50 };
	const CnOpaqueColor white = cnOpaqueColor_MakeRGBu8(255, 255, 255);
	cnR_DrawDebugRect(squareAnim.position, rectSize, white);

	cnR_EndFrame();
}

CN_GAME_API void CnPlugin_Tick(CnTime dt)
{
	CnInput* input = cnInput_Poll();
	CN_ASSERT(input, "CnInput poll provided a null pointer.");

	// TODO: Add to CnInput detecting "un-reset" keys.
	// Input_FreshlyPressed(keyname)
	static bool spaceProcessed = false;
	const bool spaceDown = cnKeySet_Contains(&input->keySet.down, SDLK_SPACE);

	if (!squareAnim.transitioning) {
		if (spaceDown && !spaceProcessed) {
			Anim_Start(&squareAnim);
			spaceProcessed = true;
		}
	}
	else {
		Anim_Update(&squareAnim, dt);
		if (spaceDown && !spaceProcessed) {
			Anim_Reverse(&squareAnim);
			spaceProcessed = true;
		}
	}

	if (!spaceDown) {
		spaceProcessed = false;
	}
}

CN_GAME_API void CnPlugin_Shutdown(void)
{
}
