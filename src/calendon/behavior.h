#ifndef CN_BEHAVIOR_H
#define CN_BEHAVIOR_H

/**
 * Behaviors group frame update functions, to be used by systems to group
 * their runtime behaviors.
 */

#include <calendon/cn.h>

#include <calendon/shared-library.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Data to be used within frame functions.  Using a single type for all of the
 * frame type of functions making these functions consistent and interchangable.
 */
typedef struct {
	CnTime dt;
} CnFrameEvent;

/**
 * Functions called during the various parts of a frame update.
 */
typedef void (*CnBehavior_FrameFn)(CnFrameEvent* event);

/**
 * A group of functions related to the various parts of a frame update.
 */
typedef struct {
	// Frame multi-phase update ticks.
	CnBehavior_FrameFn beginFrame;
	CnBehavior_FrameFn tick;
	CnBehavior_FrameFn draw;
	CnBehavior_FrameFn endFrame;
} CnBehavior;

// Convenience functions provided to more simply and safely call frame functions
// on a behavior, since a behavior may not have all frame functions assigned.
void cnBehavior_BeginFrame(CnBehavior* behavior, CnFrameEvent* event);
void cnBehavior_Tick(CnBehavior* behavior, CnFrameEvent* event);
void cnBehavior_Draw(CnBehavior* behavior, CnFrameEvent* event);
void cnBehavior_EndFrame(CnBehavior* behavior, CnFrameEvent* event);

#ifdef __cplusplus
}
#endif

#endif /* CN_BEHAVIOR_H */
