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


typedef struct {
	// Current position.
	CnFloat2 position;

	bool transitioning;
	float t;  // current interpolation value
	uint64_t elapsed; // elapsed time, used to determine t

	// Current state of the animation.
	// need some sort of indication of which state being transitioned to.
	CnFloat2* last, *next;
} BinaryAnimation;

void anim_start(BinaryAnimation* anim)
{
	if (!anim->transitioning) {
		// Move to the next state transition.
		CnFloat2* temp = anim->last;
		anim->last = anim->next;
		anim->next = temp;

		anim->transitioning = true;
		anim->elapsed = (uint64_t) 0;
		anim->t = 0.0f;
		anim->position = cnFloat2_Add(cnFloat2_Multiply(*anim->last, 1.0f - anim->t),
									  cnFloat2_Multiply(*anim->next, anim->t));
	}
}

void anim_update(BinaryAnimation* anim, uint64_t dt, uint64_t rate)
{
	if (anim->transitioning) {
		anim->elapsed += dt;
		anim->elapsed = anim->elapsed < rate ? anim->elapsed : rate;
		anim->t = (1.0f * anim->elapsed / rate); // puts t in [0, 1];
		anim->t = fmin(1.0f, fmax(anim->t, 0.0f));
		CN_ASSERT(0.0f <= anim->t && anim->t <= 1.0f, "Interpolation t is not in range [0, 1]");
		anim->position = cnFloat2_Add(cnFloat2_Multiply(*anim->last, 1.0f - anim->t),
									  cnFloat2_Multiply(*anim->next, anim->t));
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

CnFloat2 left, right;
BinaryAnimation squareAnim;

CN_GAME_API bool CnPlugin_Init(void)
{
	cnLog_RegisterSystem(&LogSysSample, "Sample", CN_LOG_TRACE);
	CN_TRACE(LogSysSample, "Sample loaded");

	left = cnFloat2_Make(300, 300);
	right = cnFloat2_Make(500, 300);

	squareAnim.position = left;
	squareAnim.t = 0.0f;
	squareAnim.elapsed = 0;
	squareAnim.last = &right;
	squareAnim.next = &left;
	squareAnim.transitioning = false;
	return true;
}

CN_GAME_API void CnPlugin_Draw(void)
{
	cnR_StartFrame();

	CnDimension2f rectSize = { 50, 50 };
	CnRGB8u white = { 255, 255, 255 };
	cnR_DrawDebugRect(squareAnim.position, rectSize, white);

	cnR_EndFrame();
}

CN_GAME_API void CnPlugin_Tick(uint64_t dt)
{
	CnInput* input = cnUI_InputPoll();
	CN_ASSERT(input, "CnInput poll provided a null pointer.");

	const uint64_t rate = cnTime_MsToNs(400);
	if (!squareAnim.transitioning) {
		if (cnKeySet_Contains(&input->keySet.down, SDLK_SPACE)) {
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

CN_GAME_API void CnPlugin_Shutdown(void)
{
}
