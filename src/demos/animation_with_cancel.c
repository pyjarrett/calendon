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
static uint64_t animationRate;

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

void anim_update(BinaryAnimation* anim, uint64_t dt, uint64_t animationRate)
{
	if (anim->transitioning) {
		anim->elapsed += dt;
		anim->elapsed = anim->elapsed < animationRate ? anim->elapsed : animationRate;
		anim->t = (1.0f * anim->elapsed / animationRate); // puts t in [0, 1];
		anim->t = cnFloat_Clamp(anim->t, 0.0f, 1.0f);
		CN_ASSERT(0.0f <= anim->t && anim->t <= 1.0f, "Interpolation t is not in range [0, 1]");
		anim->position = cnFloat2_Add(cnFloat2_Multiply(*anim->last, 1.0f - anim->t),
									  cnFloat2_Multiply(*anim->next, anim->t));
	}
}

void anim_complete(BinaryAnimation* anim, uint64_t animationRate)
{
	if (anim->transitioning && anim->elapsed >= animationRate) {
		anim->elapsed = animationRate;
		anim->t = (1.0f * anim->elapsed / animationRate); // puts t in [0, 1];
		anim->transitioning = false;
	}
}

CnFloat2 left, right;
BinaryAnimation squareAnim;

void applyButtonMapping(const CnInput* input, CnButtonMapping* mapping)
{
	CN_ASSERT_NOT_NULL(input);
	CN_ASSERT_NOT_NULL(mapping);

	for (uint32_t i = 0; i < input->keySet.down.size; ++i) {
		const CnPhysicalButtonId buttonId = input->keySet.down.keys[i];
		if (cnButtonMapping_IsMapped(mapping, buttonId)) {
			CnDigitalButton* button = cnButtonMapping_LookUp(mapping, buttonId);
			CN_ASSERT_NOT_NULL(button);
			cnDigitalButton_Press(button);
		}
	}

	for (uint32_t i = 0; i < input->keySet.up.size; ++i) {
		const CnPhysicalButtonId buttonId = input->keySet.up.keys[i];
		if (cnButtonMapping_IsMapped(mapping, buttonId)) {
			CnDigitalButton* button = cnButtonMapping_LookUp(mapping, buttonId);
			CN_ASSERT_NOT_NULL(button);
			cnDigitalButton_Release(button);
		}
	}
}

void applyInputs(const CnInput* input, const uint64_t dt)
{
	CN_ASSERT_NOT_NULL(input);

	// Apply the tick before applying inputs.
	cnAction_Tick(&changeAction, dt);

	applyButtonMapping(input, &buttonMapping);

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
	squareAnim.elapsed = 0;
	squareAnim.last = &right;
	squareAnim.next = &left;
	squareAnim.transitioning = false;
	animationRate = cnTime_MsToNs(2000);

	cnDigitalButton_Set(&startButton, CnDigitalButtonStateUp);
	cnButtonMapping_Clear(&buttonMapping);
	cnButtonMapping_Map(&buttonMapping, SDLK_SPACE, &startButton);

	cnAction_Set(&changeAction,
	cnTime_MsToNs(1000),
	animationRate,
	cnTime_MsToNs(2000));
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
			sprintf(buffer, "In Progress: %" PRIu64, changeAction.executionTimeLeft);
			break;
		case CnActionStateCoolingDown:
			sprintf(buffer, "Cooling Down: %" PRIu64, changeAction.coolDownLeft);
			break;
		case CnActionStateWindingUp:
			sprintf(buffer, "Winding Up: %" PRIu64, changeAction.windUpLeft);
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

CN_GAME_API void CnPlugin_Tick(uint64_t dt)
{
	CnInput* input = cnUI_InputPoll();
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
