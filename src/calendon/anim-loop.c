#include "anim-loop.h"

CnAnimationStateIndex AnimLoop_NextState(CnAnimationLoop* loop, uint64_t current)
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

	return  (float)cursor->elapsed / loop->elapsed[cursor->current];
}

/**
 * Using an animation loop, update the animation cursor to the next stage
 * if necessary.
 */
void cnAnimLoop_Tick(CnAnimationLoop* loop, CnAnimationLoopCursor* cursor, uint64_t dt)
{
	CN_ASSERT(loop != NULL, "Cannot update a null animation loop");
	CN_ASSERT(cursor != NULL, "Cannot update a null animation cursor");

	cursor->elapsed += dt;
	if (cursor->elapsed >= loop->elapsed[cursor->current]) {
		cursor->current = AnimLoop_NextState(loop, cursor->current);
		cursor->elapsed = 0;
	}
}
