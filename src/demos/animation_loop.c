/*
 * Working with animation loops.
 */
#include <knell/kn.h>
#include <knell/log.h>
#include <knell/math2.h>
#include <knell/render.h>
#include <knell/time.h>

LogHandle LogSysSample;

typedef uint32_t AnimationStateId;

#define KN_MAX_ANIMATION_STATES 16
/*
 * Animation is split between the definition of the cycle itself, and individual
 * instances (cursors) of animation within that cycle.
 */
typedef struct {
	AnimationStateId current;
	uint64_t elapsed;
} AnimationCursor;

/**
 * Animations really don't care about what data they are accessing.  A timed
 * animation loop could access whatever data from an array, so the animation
 * state doesn't concern itself with that.
 */
typedef struct {
	// elapsed[i] is the amount of time spent in "i" before transitioning
	uint64_t elapsed[KN_MAX_ANIMATION_STATES];
	AnimationStateId numStates;
} AnimationLoop;

static AnimationStateId AnimLoop_NextState(AnimationLoop* loop, uint64_t current)
{
	KN_ASSERT(loop != NULL, "Cannot update a null animation loop");
	return (AnimationStateId)((current + 1) % loop->numStates);
}

/**
 * Calculates the fraction of the transition which is complete for the cursor.
 */
float AnimLoop_CalcAlpha(AnimationLoop* loop, AnimationCursor* cursor)
{
	KN_ASSERT(loop != NULL, "Cannot update a null animation loop");
	KN_ASSERT(cursor != NULL, "Cannot update a null animation cursor");

	KN_ASSERT(cursor->current < loop->numStates, "Cursor %" PRIu32 " out of loop"
		"state index bounds %" PRIu64, cursor->current, loop->numStates);

	return  (float)cursor->elapsed / loop->elapsed[cursor->current];
}

/**
 * Using an animation loop, update the animation cursor to the next stage
 * if necessary.
 */
void Anim_Tick(AnimationLoop* loop, AnimationCursor* cursor, uint64_t dt)
{
	KN_ASSERT(loop != NULL, "Cannot update a null animation loop");
	KN_ASSERT(cursor != NULL, "Cannot update a null animation cursor");

	cursor->elapsed += dt;
	if (cursor->elapsed >= loop->elapsed[cursor->current]) {
		cursor->current = AnimLoop_NextState(loop, cursor->current);
		cursor->elapsed = 0;
	}
}

#define SAMPLE_POINTS 4
AnimationCursor sampleCursor;
AnimationLoop sampleLoop;
float2 points[SAMPLE_POINTS];

KN_GAME_API void Game_Init(void)
{
	Log_RegisterSystem(&LogSysSample, "Sample", KN_LOG_TRACE);
	KN_TRACE(LogSysSample, "Sample loaded");

	sampleLoop.numStates = SAMPLE_POINTS;
	sampleLoop.elapsed[0] = Time_MsToNs(400);
	sampleLoop.elapsed[1] = Time_MsToNs(1000);
	sampleLoop.elapsed[2] = Time_MsToNs(2000);
	sampleLoop.elapsed[3] = Time_MsToNs(500);

	points[0] = float2_Make(200, 200);
	points[1] = float2_Make(400, 200);
	points[2] = float2_Make(400, 400);
	points[3] = float2_Make(200, 400);
}

KN_GAME_API void Game_Draw(void)
{
	R_StartFrame();

	dimension2f rectSize = { 50, 50 };
	rgb8 white = { 255, 255, 255 };

	float2 animatedPosition = float2_Lerp(points[sampleCursor.current],
		points[AnimLoop_NextState(&sampleLoop, sampleCursor.current)],
		AnimLoop_CalcAlpha(&sampleLoop, &sampleCursor));
	R_DrawDebugRect(animatedPosition, rectSize, white);

	R_EndFrame();
}

KN_GAME_API void Game_Tick(uint64_t dt)
{
	Anim_Tick(&sampleLoop, &sampleCursor, dt);
}

KN_GAME_API void Game_Shutdown()
{
}
