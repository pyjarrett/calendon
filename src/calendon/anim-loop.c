#include "anim-loop.h"

CnAnimationStateIndex AnimLoop_NextState(CnAnimationLoop* loop, CnAnimationStateIndex current)
{
	CN_ASSERT(loop != NULL, "Cannot update a null animation loop");
	return (CnAnimationStateIndex)((current + 1) % loop->numStates);
}

/**
 * Calculates the fraction of the transition which is complete for the cursor.
 */
float cnAnimLoop_CalcAlpha(CnAnimationLoop* loop, CnAnimationLoopCursor* cursor)
{
	CN_ASSERT(loop != NULL, "Cannot update a null animation loop");
	CN_ASSERT(cursor != NULL, "Cannot update a null animation cursor");
	CN_ASSERT(loop->numStates < CN_MAX_ANIMATION_STATES, "Animation loop has"
		"more states %" PRIu32 "than allowed %" PRIu32, loop->numStates,
		CN_MAX_ANIMATION_STATES);

	CN_ASSERT(cursor->current < loop->numStates, "Cursor %" PRIu32 " out of loop"
		"state index bounds %" PRIu32, cursor->current, loop->numStates);

	return cnTime_Lerp(cursor->elapsed, loop->elapsed[cursor->current]);
}

/**
 * Using an animation loop, update the animation cursor to the next stage
 * if necessary.
 */
void cnAnimLoop_Tick(CnAnimationLoop* loop, CnAnimationLoopCursor* cursor, CnTime dt)
{
	CN_ASSERT(loop != NULL, "Cannot update a null animation loop");
	CN_ASSERT(cursor != NULL, "Cannot update a null animation cursor");

	cursor->elapsed = cnTime_Add(cursor->elapsed, dt);
	if (cnTime_LessThan(loop->elapsed[cursor->current], cursor->elapsed)) {
		cursor->current = AnimLoop_NextState(loop, cursor->current);
		cursor->elapsed = cnTime_MakeZero();
	}
}
