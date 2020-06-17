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

void cnAction_Set(CnAction* action, uint64_t windUp, uint64_t executionTime, uint64_t coolDown)
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
			if (action->windUp > 0) {
				action->windUpLeft = action->windUp;
				action->state = CnActionStateWindingUp;
			}
			else {
				cnAction_Enter(action, CnActionStateInProgress);
			}
			break;
		case CnActionStateInProgress:
			if (action->executionTime > 0) {
				action->executionTimeLeft = action->executionTime;
				action->state = CnActionStateInProgress;
			}
			else {
				cnAction_Enter(action, CnActionStateCoolingDown);
			}
			break;
		case CnActionStateCoolingDown:
			if (action->requireManualRequest || action->coolDown > 0) {
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

void cnAction_Tick(CnAction* action, uint64_t dt)
{
	CN_ASSERT_NOT_NULL(action);
	switch (action->state) {
		case CnActionStateReady:
			break;
		case CnActionStateWindingUp:
			action->windUpLeft = cnTime_MonotonicSubtract(action->windUpLeft, dt);
			if (action->windUpLeft == 0) {
				cnAction_Enter(action, CnActionStateInProgress);
			}
			break;
		case CnActionStateInProgress:
			action->executionTimeLeft = cnTime_MonotonicSubtract(action->executionTimeLeft, dt);
			if (action->executionTimeLeft == 0) {
				cnAction_Enter(action, CnActionStateCoolingDown);
			}
			break;
		case CnActionStateCoolingDown:
			action->coolDownLeft = cnTime_MonotonicSubtract(action->coolDownLeft, dt);
			if (action->coolDownLeft == 0 && !action->requireManualRequest) {
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
		action->windUpLeft, action->windUp,
		action->executionTimeLeft, action->executionTime,
		action->coolDownLeft, action->coolDown,
		action->requireManualRequest ? "manual" : "auto");
}
