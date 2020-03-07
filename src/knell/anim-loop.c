#include "anim-loop.h"

KN_API AnimationStateIndex AnimLoop_NextState(AnimationLoop* loop, uint64_t current)
{
	KN_ASSERT(loop != NULL, "Cannot update a null animation loop");
	return (AnimationStateIndex)((current + 1) % loop->numStates);
}

/**
 * Calculates the fraction of the transition which is complete for the cursor.
 */
KN_API float AnimLoop_CalcAlpha(AnimationLoop* loop, AnimationLoopCursor* cursor)
{
	KN_ASSERT(loop != NULL, "Cannot update a null animation loop");
	KN_ASSERT(cursor != NULL, "Cannot update a null animation cursor");
	KN_ASSERT(loop->numStates < KN_MAX_ANIMATION_STATES, "Animation loop has"
		"more states %" PRIu32 "than allowed %" PRIu32, loop->numStates,
		KN_MAX_ANIMATION_STATES);

	KN_ASSERT(cursor->current < loop->numStates, "Cursor %" PRIu32 " out of loop"
		"state index bounds %" PRIu32, cursor->current, loop->numStates);

	return  (float)cursor->elapsed / loop->elapsed[cursor->current];
}

/**
 * Using an animation loop, update the animation cursor to the next stage
 * if necessary.
 */
KN_API void AnimLoop_Tick(AnimationLoop* loop, AnimationLoopCursor* cursor, uint64_t dt)
{
	KN_ASSERT(loop != NULL, "Cannot update a null animation loop");
	KN_ASSERT(cursor != NULL, "Cannot update a null animation cursor");

	cursor->elapsed += dt;
	if (cursor->elapsed >= loop->elapsed[cursor->current]) {
		cursor->current = AnimLoop_NextState(loop, cursor->current);
		cursor->elapsed = 0;
	}
}
