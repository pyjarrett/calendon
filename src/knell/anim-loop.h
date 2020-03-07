#ifndef KN_ANIM_LOOP_H
#define KN_ANIM_LOOP_H

/*
 * Animation loop controls.  Animations don't care about the underlying data
 * types, so these could also be thought of as a FSM which loops through
 * timed states.
 */

#include <knell/kn.h>

typedef uint32_t AnimationStateIndex;

/**
 * Bound the maximum number of states to something reasonable.
 */
#define KN_MAX_ANIMATION_STATES 16

/*
 * Animation is split between the definition of the cycle itself, and individual
 * instances (cursors) of animation within that cycle.
 *
 * An animation cursor denotes the current state of a single instance of an
 * animation.  This doesn't reference the AnimationLoop itself, to reduce the
 * amount of state carried around, and also allows for cursors to be swapped
 * to another loop.
 */
typedef struct {
	AnimationStateIndex current;
	uint64_t elapsed;
} AnimationLoopCursor;

/**
 * Animations really don't care about what data they are accessing.  A timed
 * animation loop could access whatever data from an array, so the animation
 * state doesn't concern itself with that.
 */
typedef struct {
	// elapsed[i] is the amount of time spent in "i" before transitioning
	uint64_t elapsed[KN_MAX_ANIMATION_STATES];
	AnimationStateIndex numStates;
} AnimationLoop;

KN_API float AnimLoop_CalcAlpha(AnimationLoop* loop, AnimationLoopCursor* cursor);
KN_API AnimationStateIndex AnimLoop_NextState(AnimationLoop* loop, uint64_t current);
KN_API void AnimLoop_Tick(AnimationLoop* loop, AnimationLoopCursor* cursor, uint64_t dt);

#endif /* KN_ANIM_LOOP_H */
