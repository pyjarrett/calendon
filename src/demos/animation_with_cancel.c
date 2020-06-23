/*
 * Used to experiment with animation between discrete steps.
 */
#include <calendon/cn.h>

#include <calendon/action.h>
#include <calendon/assets.h>
#include <calendon/float.h>
#include <calendon/input-button-mapping.h>
#include <calendon/input-digital-button.h>
#include <calendon/input-keyset.h>
#include <calendon/log.h>
#include <calendon/math2.h>
#include <calendon/path.h>
#include <calendon/render.h>
#include <calendon/time.h>
#include <calendon/ui.h>

#include <math.h>

CnLogHandle LogSysSample;
CnButtonMapping buttonMapping;
CnDigitalButton startButton;
CnAction changeAction;
CnFontId font;
static CnTime animationRate;

typedef struct {
	// Current position.
	CnFloat2 position;

	bool transitioning;
	float t;  // current interpolation value
	CnTime elapsed; // elapsed time, used to determine t

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
		anim->elapsed = cnTime_MakeZero();
		anim->t = 0.0f;
		anim->position = cnFloat2_Add(cnFloat2_Multiply(*anim->last, 1.0f - anim->t),
									  cnFloat2_Multiply(*anim->next, anim->t));
	}
}

void anim_update(BinaryAnimation* anim, CnTime dt, CnTime animationRate)
{
	if (anim->transitioning) {
		anim->elapsed = cnTime_Add(anim->elapsed, dt);
		anim->elapsed = cnTime_Min(anim->elapsed, animationRate);
		anim->t = cnTime_Lerp(anim->elapsed, animationRate);
		CN_ASSERT(0.0f <= anim->t && anim->t <= 1.0f, "Interpolation t is not in range [0, 1]");
		anim->position = cnFloat2_Add(cnFloat2_Multiply(*anim->last, 1.0f - anim->t),
									  cnFloat2_Multiply(*anim->next, anim->t));
	}
}

void anim_complete(BinaryAnimation* anim, CnTime animationRate)
{
	if (anim->transitioning && !cnTime_LessThan(anim->elapsed, animationRate)) {
		anim->elapsed = animationRate;
		anim->t = cnTime_Lerp(anim->elapsed, animationRate);
		anim->transitioning = false;
	}
}

CnFloat2 left, right;
BinaryAnimation squareAnim;

void applyInputs(const CnInput* input, const CnTime dt)
{
	CN_ASSERT_NOT_NULL(input);

	// Apply the tick before applying inputs.
	cnAction_Tick(&changeAction, dt);
	cnInput_ApplyButtonMapping(input, &buttonMapping);

	if (cnDigitalButton_IsDown(&startButton)) {
		cnAction_Start(&changeAction);
	}
	else {
		cnAction_Cancel(&changeAction);
	}
}

CN_GAME_API bool CnPlugin_Init(void)
{
	cnLog_RegisterSystem(&LogSysSample, "Sample", CnLogVerbosityTrace);
	CN_TRACE(LogSysSample, "Sample loaded");

	CnPathBuffer fontPath;
	cnAssets_PathBufferFor("fonts/bizcat.psf", &fontPath);

	cnR_CreateFont(&font);
	if (!cnR_LoadPSF2Font(font, fontPath.str))	{
		CN_FATAL_ERROR("Unable to load font: %s", fontPath.str);
	}

	left = cnFloat2_Make(300, 300);
	right = cnFloat2_Make(500, 300);

	squareAnim.position = left;
	squareAnim.t = 0.0f;
	squareAnim.elapsed = cnTime_MakeZero();
	squareAnim.last = &right;
	squareAnim.next = &left;
	squareAnim.transitioning = false;
	animationRate = cnTime_MakeMilli(2000);

	cnDigitalButton_Set(&startButton, CnDigitalButtonStateUp);
	cnButtonMapping_Clear(&buttonMapping);
	cnButtonMapping_Map(&buttonMapping, SDLK_SPACE, &startButton);

	cnAction_Set(&changeAction,
		cnTime_MakeMilli(1000),
	animationRate,
		cnTime_MakeMilli(2000));
	cnAction_Reset(&changeAction);
	return true;
}

void drawUI(void)
{
	CnFloat2 statusLocation = cnFloat2_Make(50, 50);
	char buffer[1024];
	switch (changeAction.state) {
		case CnActionStateReady:
			sprintf(buffer, "Ready: Press Space");
			break;
		case CnActionStateInProgress:
			sprintf(buffer, "In Progress: %" PRIu64, cnTime_Milli(changeAction.executionTimeLeft));
			break;
		case CnActionStateCoolingDown:
			sprintf(buffer, "Cooling Down: %" PRIu64, cnTime_Milli(changeAction.coolDownLeft));
			break;
		case CnActionStateWindingUp:
			sprintf(buffer, "Winding Up: %" PRIu64, cnTime_Milli(changeAction.windUpLeft));
			break;
		default: CN_ASSERT(false, "Unknown action state");
	}
	cnR_DrawSimpleText(font, statusLocation, buffer);
}

CN_GAME_API void CnPlugin_Draw(void)
{
	cnR_StartFrame();

	CnDimension2f rectSize = { 50, 50 };
	CnRGB8u white = { 255, 255, 255 };
	cnR_DrawDebugRect(squareAnim.position, rectSize, white);

	drawUI();

	cnR_EndFrame();
}

CN_GAME_API void CnPlugin_Tick(CnTime dt)
{
	CnInput* input = cnInput_Poll();
	CN_ASSERT_NOT_NULL(input);

	applyInputs(input, dt);

	if (!squareAnim.transitioning) {
		if (changeAction.state == CnActionStateInProgress) {
			anim_start(&squareAnim);
		}
	}
	else {
		anim_update(&squareAnim, dt, animationRate);
	}
	anim_complete(&squareAnim, animationRate);
}

CN_GAME_API void CnPlugin_Shutdown(void)
{
}
