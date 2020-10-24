#include "behavior.h"

/**
 * Loads a plugin's shared library and assigns its functions from a file.
 */
bool cnBehavior_LoadFromSharedLibrary(CnBehavior* behavior, CnSharedLibrary library)
{
	CN_ASSERT_PTR(behavior);
	CN_ASSERT_PTR(library);

	// TODO: Replace CnPlugin with a given behavior name to find.


	behavior->beginFrame = (CnBehavior_FrameFn) cnSharedLibrary_LookupFn(library, "CnPlugin_BeginFrame");
	behavior->endFrame   = (CnBehavior_FrameFn) cnSharedLibrary_LookupFn(library, "CnPlugin_EndFrame");
	behavior->draw       = (CnBehavior_FrameFn) cnSharedLibrary_LookupFn(library, "CnPlugin_Draw");
	behavior->tick       = (CnBehavior_FrameFn) cnSharedLibrary_LookupFn(library, "CnPlugin_Tick");

	return true;
}

/**
 * A "complete" behavior has all function pointers assigned.
 *
 * @todo Replace with cnBehavior_BeginFrame(behvaior) and similar functions.
 */
bool cnBehavior_IsComplete(CnBehavior* behavior)
{
	CN_ASSERT_PTR(behavior);
	return behavior->beginFrame
		&& behavior->endFrame
		&& behavior->draw
		&& behavior->tick;
}

void cnBehavior_BeginFrame(CnBehavior* behavior, CnFrameEvent* event)
{
	CN_ASSERT_PTR(behavior);
	CN_ASSERT_PTR(event);
	if (behavior) {
		behavior->beginFrame(event);
	}
}

void cnBehavior_Tick(CnBehavior* behavior, CnFrameEvent* event)
{
	CN_ASSERT_PTR(behavior);
	CN_ASSERT_PTR(event);
	if (behavior) {
		behavior->tick(event);
	}
}

void cnBehavior_Draw(CnBehavior* behavior, CnFrameEvent* event)
{
	CN_ASSERT_PTR(behavior);
	CN_ASSERT_PTR(event);
	if (behavior) {
		behavior->draw(event);
	}
}

void cnBehavior_EndFrame(CnBehavior* behavior, CnFrameEvent* event)
{
	CN_ASSERT_PTR(behavior);
	CN_ASSERT_PTR(event);
	if (behavior) {
		behavior->endFrame(event);
	}
}