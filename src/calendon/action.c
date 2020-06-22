#include "action.h"
#include "log.h"

#include <calendon/time.h>

const char* CnActionState_AsString(CnActionState state)
{
	switch (state) {
		case CnActionStateReady:       return "CnActionStateReady";
		case CnActionStateWindingUp:   return "CnActionStateWindingUp";
		case CnActionStateInProgress:  return "CnActionStateInProgress";
		case CnActionStateCoolingDown: return "CnActionStateCoolingDown";
		default:                       return "Unknown CnActionState";
	}
}

void cnAction_Set(CnAction* action, CnTime windUp,
	CnTime executionTime, CnTime coolDown)
{
	CN_ASSERT_NOT_NULL(action);
	action->windUp = windUp;
	action->executionTime = executionTime;
	action->coolDown = coolDown;
	action->requireManualRequest = false;
}

/**
 * Called to place an action into a given state.
 */
static void cnAction_Enter(CnAction* action, CnActionState state)
{
	CN_ASSERT_NOT_NULL(action);
	switch (state) {
		case CnActionStateReady:
			action->state = CnActionStateReady;
			break;
		case CnActionStateWindingUp:
			if (!cnTime_IsZero(action->windUp)) {
				action->windUpLeft = action->windUp;
				action->state = CnActionStateWindingUp;
			}
			else {
				cnAction_Enter(action, CnActionStateInProgress);
			}
			break;
		case CnActionStateInProgress:
			if (!cnTime_IsZero(action->executionTime)) {
				action->executionTimeLeft = action->executionTime;
				action->state = CnActionStateInProgress;
			}
			else {
				cnAction_Enter(action, CnActionStateCoolingDown);
			}
			break;
		case CnActionStateCoolingDown:
			if (!cnTime_IsZero(action->coolDown) || action->requireManualRequest) {
				action->coolDownLeft = action->coolDown;
				action->state = CnActionStateCoolingDown;
			}
			else {
				cnAction_Enter(action, CnActionStateReady);
			}
			break;
		default:
			CN_ASSERT(false, "Invalid action state.");
	}
}

void cnAction_Start(CnAction* action)
{
	CN_ASSERT_NOT_NULL(action);
	switch (action->state) {
		case CnActionStateReady:
			cnAction_Enter(action, CnActionStateWindingUp);
			break;
		case CnActionStateWindingUp:
			break;
		case CnActionStateInProgress:
			break;
		case CnActionStateCoolingDown:
			break;
		default:
			CN_ASSERT(false, "Invalid action state.");
	}
}

void cnAction_Cancel(CnAction* action)
{
	CN_ASSERT_NOT_NULL(action);
	switch (action->state) {
		case CnActionStateReady:
			break;
		case CnActionStateWindingUp:
			cnAction_Enter(action, CnActionStateReady);
			break;
		case CnActionStateInProgress:
			break;
		case CnActionStateCoolingDown:
			break;
		default:
			CN_ASSERT(false, "Invalid action state.");
	}
}

void cnAction_Tick(CnAction* action, CnTime dt)
{
	CN_ASSERT_NOT_NULL(action);
	switch (action->state) {
		case CnActionStateReady:
			break;
		case CnActionStateWindingUp:
			action->windUpLeft = cnTime_SubtractMonotonic(action->windUpLeft, dt);
			if (cnTime_IsZero(action->windUpLeft)) {
				cnAction_Enter(action, CnActionStateInProgress);
			}
			break;
		case CnActionStateInProgress:
			action->executionTimeLeft = cnTime_SubtractMonotonic(action->executionTimeLeft, dt);
			if (cnTime_IsZero(action->executionTimeLeft)) {
				cnAction_Enter(action, CnActionStateCoolingDown);
			}
			break;
		case CnActionStateCoolingDown:
			action->coolDownLeft = cnTime_SubtractMonotonic(action->coolDownLeft, dt);
			if (cnTime_IsZero(action->coolDownLeft) && !action->requireManualRequest) {
				cnAction_Enter(action, CnActionStateReady);
			}
			break;
		default:
			CN_ASSERT(false, "Invalid action state.");
	}
}

void cnAction_Reset(CnAction* action)
{
	CN_ASSERT_NOT_NULL(action);
	switch (action->state) {
		case CnActionStateReady:
			break;
		case CnActionStateWindingUp:
			break;
		case CnActionStateInProgress:
			break;
		case CnActionStateCoolingDown:
			cnAction_Enter(action, CnActionStateReady);
			break;
		default:
			CN_ASSERT(false, "Invalid action state.");
	}
}

void cnAction_Log(CnAction* action, CnLogHandle log)
{
	CN_ASSERT_NOT_NULL(action);
	CN_TRACE(log, "%s: %" PRIu64 "/%" PRIu64
			" %" PRIu64 "/%" PRIu64
			" %" PRIu64 "/%" PRIu64
			" %s",
		CnActionState_AsString(action->state),
		cnTime_Milli(action->windUpLeft),
		cnTime_Milli(action->windUp),
		cnTime_Milli(action->executionTimeLeft),
		cnTime_Milli(action->executionTime),
		cnTime_Milli(action->coolDownLeft),
		cnTime_Milli(action->coolDown),
		action->requireManualRequest ? "manual" : "auto");
}
