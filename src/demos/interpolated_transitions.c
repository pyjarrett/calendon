/*
 * Used to experiment with animation between discrete steps.
 */
#include <knell/kn.h>
#include <knell/input.h>
#include <knell/log.h>
#include <knell/math2.h>
#include <knell/render.h>
#include <knell/time.h>
#include <knell/ui.h>

#include <math.h>

LogHandle LogSysSample;

/**
 * Stores the current state of an animation.
 *
 * This probably should be divided into multiple types.
 */
typedef struct {
	// Current position.
	float2 position;

	// Is this animation currently active?
	bool transitioning;

	// elapsed time, used to determine t, based on the animation rate
	uint64_t elapsed;

	// The time it takes to change states.
	uint64_t rate;

	// current interpolation value
	float t;

	// Current state of the animation.
	// need some sort of indication of which state being transitioned to.
	// TODO: Convert to a Transform2
	float2* last, *next;
} BinaryAnimation;

void Anim_Start(BinaryAnimation* anim)
{
	if (!anim->transitioning) {
		// Move to the next state transition.
		float2* temp = anim->last;
		anim->last = anim->next;
		anim->next = temp;

		anim->transitioning = true;
		anim->elapsed = (uint64_t)0;
		anim->rate = Time_MsToNs(2000);
		anim->t = 0.0f;
		anim->position = float2_Add(float2_Multiply(*anim->last, 1.0f - anim->t), float2_Multiply(*anim->next, anim->t));
	}
}

/**
 * Cause an existing animation in transition to reverse directions.
 */
void Anim_Reverse(BinaryAnimation* anim)
{
	if (!anim->transitioning) return;

	float2* temp = anim->last;
	anim->last = anim->next;
	anim->next = temp;

	anim->t = 1.0f - anim->t;
	anim->elapsed = anim->rate - anim->elapsed;

	// Position should remain the same for this frame.
}

/**
 * Recalculates an animation position based upon its current "t" value.
 */
void Anim_Recalculate(BinaryAnimation *anim)
{
	anim->position = float2_Add(float2_Multiply(*anim->last, 1.0f - anim->t), float2_Multiply(*anim->next, anim->t));
}

void Anim_Complete(BinaryAnimation* anim)
{
	if (anim->transitioning && anim->elapsed >= anim->rate) {
		anim->elapsed = anim->rate;
		anim->t = (1.0f * anim->elapsed / anim->rate); // puts t in [0, 1];
		anim->transitioning = false;
	}
}

void Anim_Update(BinaryAnimation* anim, uint64_t dt)
{
	if (anim->transitioning) {
		anim->elapsed += dt;
		anim->elapsed = (uint64_t)fminf((float)anim->elapsed, (float)anim->rate);
		anim->t = (1.0f * (float)anim->elapsed / (float)anim->rate); // puts t in [0, 1];
		anim->t = fminf(1.0f, fmaxf(anim->t, 0.0f));
		KN_ASSERT(0.0f <= anim->t && anim->t <= 1.0f, "Interpolation t is not in range [0, 1]");
		Anim_Recalculate(anim);
		Anim_Complete(anim);
	}
}

float2 left, right;
BinaryAnimation squareAnim;

KN_GAME_API bool Plugin_Init(void)
{
	Log_RegisterSystem(&LogSysSample, "Sample", KN_LOG_TRACE);
	KN_TRACE(LogSysSample, "Sample loaded");

	left = float2_Make(300, 300);
	right = float2_Make(500, 300);

	squareAnim.position = left;
	squareAnim.t = 0.0f;
	squareAnim.elapsed = 0;
	squareAnim.last = &right;
	squareAnim.next = &left;
	squareAnim.transitioning = false;
	return true;
}

KN_GAME_API void Plugin_Draw(void)
{
	R_StartFrame();

	Dimension2f rectSize = { 50, 50 };
	rgb8 white = { 255, 255, 255 };
	R_DrawDebugRect(squareAnim.position, rectSize, white);

	R_EndFrame();
}

KN_GAME_API void Plugin_Tick(uint64_t dt)
{
	Input* input = UI_InputPoll();
	KN_ASSERT(input, "Input poll provided a null pointer.");

	// TODO: Add to Input detecting "un-reset" keys.
	// Input_FreshlyPressed(keyname)
	static bool spaceProcessed = false;
	const bool spaceDown = KeySet_Contains(&input->keySet.down, SDLK_SPACE);

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

KN_GAME_API void Plugin_Shutdown(void)
{
}
