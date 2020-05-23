#include "float.h"

#include <math.h>

// http://c-faq.com/fp/fpequal.html
float float_RelativeDiff(const float a, const float b)
{
	const float c = fmaxf(fabsf(a), fabsf(b));
	return c == 0.0f ? 0.0f : fabsf(a - b) / c;
}
