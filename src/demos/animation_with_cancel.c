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


typedef struct {
	// Current position.
	float2 position;

	bool transitioning;
	float t;  // current interpolation value
	uint64_t elapsed; // elapsed time, used to determine t

	// Current state of the animation.
	// need some sort of indication of which state being transitioned to.
	float2* last, *next;
} BinaryAnimation;

void anim_start(BinaryAnimation* anim)
{
	if (!anim->transitioning) {
		// Move to the next state transition.
		float2* temp = anim->last;
		anim->last = anim->next;
		anim->next = temp;

		anim->transitioning = true;
		anim->elapsed = (uint64_t) 0;
		anim->t = 0.0f;
		anim->position = float2_Add(float2_Multiply(*anim->last, 1.0f - anim->t), float2_Multiply(*anim->next, anim->t));
	}
}

void anim_update(BinaryAnimation* anim, uint64_t dt, uint64_t rate)
{
	if (anim->transitioning) {
		anim->elapsed += dt;
		anim->elapsed = min(anim->elapsed, rate);
		anim->t = (1.0f * anim->elapsed / rate); // puts t in [0, 1];
		anim->t = min(1.0f, max(anim->t, 0.0f));
		KN_ASSERT(0.0f <= anim->t && anim->t <= 1.0f, "Interpolation t is not in range [0, 1]");
		anim->position = float2_Add(float2_Multiply(*anim->last, 1.0f - anim->t),
									float2_Multiply(*anim->next, anim->t));
	}
}

void anim_complete(BinaryAnimation* anim, uint64_t rate)
{
	if (anim->transitioning && anim->elapsed >= rate) {
		anim->elapsed = rate;
		anim->t = (1.0f * anim->elapsed / rate); // puts t in [0, 1];
		anim->transitioning = false;
	}
}

float2 left, right;
BinaryAnimation squareAnim;

KN_GAME_API void Game_Init(void)
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
}

KN_GAME_API void Game_Draw(void)
{
	R_StartFrame();

	Dimension2f rectSize = { 50, 50 };
	rgb8 white = { 255, 255, 255 };
	R_DrawDebugRect(squareAnim.position, rectSize, white);

	R_EndFrame();
}

KN_GAME_API void Game_Tick(uint64_t dt)
{
	Input* input = UI_InputPoll();
	KN_ASSERT(input, "Input poll provided a null pointer.");

	const uint64_t rate = Time_MsToNs(400);
	if (!squareAnim.transitioning) {
		if (KeySet_Contains(&input->keySet.down, SDLK_SPACE)) {
			anim_start(&squareAnim);
		}
	}
	else {
		anim_update(&squareAnim, dt, rate);
	}

	anim_complete(&squareAnim, rate);

	//const float pi = 3.14159f;
	//t *= (2.0f * pi); // convert to [0, 2*pi]
	//t = sinf(t);  // convert to [-1, 1]
	//t += 1; // convert to [0, 2]
	//t *= 0.5f; // convert to [0, 1]
}

KN_GAME_API void Game_Shutdown()
{
}
