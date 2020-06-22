#ifndef CN_ACTION_H
#define CN_ACTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <calendon/cn.h>
#include <calendon/log.h>
#include <calendon/time.h>

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

	CnTime windUp, windUpLeft;
	CnTime executionTime, executionTimeLeft;
	CnTime coolDown, coolDownLeft;

	/** Requires a manual reset after cooling down. */
	bool requireManualRequest;
} CnAction;

CN_API void cnAction_Set(CnAction* action, CnTime windUp, CnTime executionTime, CnTime coolDown);
CN_API void cnAction_Start(CnAction* action);
CN_API void cnAction_Cancel(CnAction* action);
CN_API void cnAction_Tick(CnAction* action, CnTime dt);
CN_API void cnAction_Reset(CnAction* action);

CN_API void cnAction_Log(CnAction* action, CnLogHandle log);

#ifdef __cplusplus
}
#endif

#endif /* CN_ACTION_H */
