/*
 * Used to experiment with animation between discrete steps.
 */
#include <knell/kn.h>
#include <knell/input.h>
#include <knell/log.h>
#include <knell/math2.h>
#include <knell/render.h>
#include <knell/time.h>

#include <math.h>
#include <knell/ui.h>

LogHandle LogSysSample;

float t;
uint64_t elapsed;

float2 position;
float2 left, right;

KN_GAME_API void Game_Init(void)
{
	Log_RegisterSystem(&LogSysSample, "Sample", KN_LOG_TRACE);
	KN_TRACE(LogSysSample, "Sample loaded");

	left = float2_Make(300, 300);
	right = float2_Make(500, 300);

	position = left;
	t = 0.0f;
	elapsed = 0;
}

KN_GAME_API void Game_Draw(void)
{
	R_StartFrame();

	dimension2f rectSize = { 50, 50 };
	rgb8 white = { 255, 255, 255};
	R_DrawDebugRect(position, rectSize, white);

	R_EndFrame();
}

KN_GAME_API void Game_Tick(uint64_t dt)
{
	KN_UNUSED(dt);
	Input* input = UI_InputPoll();
	KN_ASSERT(input, "Input poll provided a null pointer.");
	//position = float2_Make((float)input->mouse.x, (float)input->mouse.y);

	elapsed += dt;
	const uint64_t rate = Time_SecToNs(2);
	elapsed %= rate;

	const float pi = 3.14159f;
	t = (1.0f * elapsed / rate); // puts t in [0, 1];
	t *= (2.0f * pi); // convert to [0, 2*pi]
	t = sinf(t);  // convert to [-1, 1]
	t += 1; // convert to [0, 2]
	t *= 0.5f; // convert to [0, 1]

	KN_ASSERT(0.0f <= t && t <= 1.0f, "Interpolation t is not in range [0, 1]");
	position = float2_Add(float2_Multiply(left, t), float2_Multiply(right, 1.0f - t));
}

KN_GAME_API void Game_Shutdown()
{
}
