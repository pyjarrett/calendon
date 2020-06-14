#include "float.h"

#include <math.h>

// http://c-faq.com/fp/fpequal.html
float cnFloat_RelativeDiff(const float a, const float b)
{
	const float c = fmaxf(fabsf(a), fabsf(b));
	return c == 0.0f ? 0.0f : fabsf(a - b) / c;
}

float cnFloat_Clamp(const float value, const float min, const float max)
{
	CN_ASSERT_FINITE_F32(value);
	CN_ASSERT_FINITE_F32(min);
	CN_ASSERT_FINITE_F32(max);

	if (value < min) return min;
	if (value > max) return max;
	return value;
}
