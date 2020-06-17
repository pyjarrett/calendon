#ifndef CN_ACTION_H
#define CN_ACTION_H

#include <calendon/cn.h>
#include <calendon/log.h>

typedef enum {
	/**
	 * The action is available to be performed.
	 */
	CnActionStateReady,

	/**
	 * The action is started, but may still be cancelled before it begins.
	 */
	CnActionStateWindingUp,

	/**
	 * The action is currently happening.
	 */
	CnActionStateInProgress,

	 /**
	  * The action is in a cooling down period or waiting for a reset.
	  */
	 CnActionStateCoolingDown
} CnActionState;

CN_API const char* CnActionState_AsString(CnActionState state);

/**
 * Actions provide a general concept of the user performing an input to
 * accomplish something within the game.
 */
typedef struct {
	CnActionState state;

	uint64_t windUp, windUpLeft;
	uint64_t executionTime, executionTimeLeft;
	uint64_t coolDown, coolDownLeft;

	/** Requires a manual reset after cooling down. */
	bool requireManualRequest;
} CnAction;

CN_API void cnAction_Set(CnAction* action, uint64_t windUp, uint64_t executionTime, uint64_t coolDown);
CN_API void cnAction_Start(CnAction* action);
CN_API void cnAction_Cancel(CnAction* action);
CN_API void cnAction_Tick(CnAction* action, uint64_t dt);
CN_API void cnAction_Reset(CnAction* action);

CN_API void cnAction_Log(CnAction* action, CnLogHandle log);

#endif /* CN_ACTION_H */
