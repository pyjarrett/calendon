#include "math.h"

#include <stdint.h>

float4x4 float4x4_identity()
{
	return (float4x4) {{
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 }
	}};
}

float4x4 float4x4_nonuniform_scale(float x, float y, float z)
{
	return (float4x4) {{
		{ x, 0, 0, 0 }, 
		{ 0, y, 0, 0 },
		{ 0, 0, z, 0 },
		{ 0, 0, 0, 1 }
	}};
}

float4x4 float4x4_translate(float x, float y, float z)
{
	return (float4x4) {{
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ x, y, z, 1 }
	}};
}

float4x4 float4x4_multiply(float4x4 l, float4x4 r)
{
	float4x4 result;
	for (uint32_t i=0; i<4; ++i) {
		for (uint32_t j=0; j<4; ++j) {
			float element = 0.0f;
			for (uint32_t k=0; k<4; ++k) {
				// (i,j) is the element being written
				// Keep row constant for left, column constant for right.
				element += (l.m[i][k] * r.m[k][j]);
			}
			result.m[i][j] = element;
		}
	}
	return result;
}

void float4x4_debug_print(FILE* stream, float4x4 m)
{
	for (uint32_t i=0; i<4; ++i) {
		fprintf(stream, "%6.f %6.f %6.f %6.f\n", m.m[i][0], m.m[i][1], m.m[i][2], m.m[i][3]);
	}
}

