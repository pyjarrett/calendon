#include "behavior.h"

void cnBehavior_BeginFrame(CnBehavior* behavior, CnFrameEvent* event)
{
	CN_ASSERT_PTR(behavior);
	CN_ASSERT_PTR(event);
	if (behavior->beginFrame) {
		behavior->beginFrame(event);
	}
}

void cnBehavior_Tick(CnBehavior* behavior, CnFrameEvent* event)
{
	CN_ASSERT_PTR(behavior);
	CN_ASSERT_PTR(event);
	if (behavior->tick) {
		behavior->tick(event);
	}
}

void cnBehavior_Draw(CnBehavior* behavior, CnFrameEvent* event)
{
	CN_ASSERT_PTR(behavior);
	CN_ASSERT_PTR(event);
	if (behavior->draw) {
		behavior->draw(event);
	}
}

void cnBehavior_EndFrame(CnBehavior* behavior, CnFrameEvent* event)
{
	CN_ASSERT_PTR(behavior);
	CN_ASSERT_PTR(event);
	if (behavior->endFrame) {
		behavior->endFrame(event);
	}
}