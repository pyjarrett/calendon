#ifndef CN_ANIM_LOOP_H
#define CN_ANIM_LOOP_H

/*
 * Animation loop controls.  Animations don't care about the underlying data
 * types, so these could also be thought of as a FSM which loops through
 * timed states.
 */

#include <calendon/cn.h>

#include <calendon/time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t CnAnimationStateIndex;

/**
 * Bound the maximum number of states to something reasonable.
 */
#define CN_MAX_ANIMATION_STATES 16

/*
 * Animation is split between the definition of the cycle itself, and individual
 * instances (cursors) of animation within that cycle.
 *
 * An animation cursor denotes the current state of a single instance of an
 * animation.  This doesn't reference the CnAnimationLoop itself, to reduce the
 * amount of state carried around, and also allows for cursors to be swapped
 * to another loop.
 */
typedef struct {
	CnAnimationStateIndex current;
	CnTime elapsed;
} CnAnimationLoopCursor;

/**
 * Animations really don't care about what data they are accessing.  A timed
 * animation loop could access whatever data from an array, so the animation
 * state doesn't concern itself with that.
 */
typedef struct {
	// elapsed[i] is the amount of time spent in "i" before transitioning
	CnTime elapsed[CN_MAX_ANIMATION_STATES];
	CnAnimationStateIndex numStates;
} CnAnimationLoop;

CN_API float cnAnimLoop_CalcAlpha(CnAnimationLoop* loop, CnAnimationLoopCursor* cursor);
CN_API CnAnimationStateIndex AnimLoop_NextState(CnAnimationLoop* loop, CnAnimationStateIndex current);
CN_API void cnAnimLoop_Tick(CnAnimationLoop* loop, CnAnimationLoopCursor* cursor, CnTime dt);

#ifdef __cplusplus
}
#endif

#endif /* CN_ANIM_LOOP_H */
