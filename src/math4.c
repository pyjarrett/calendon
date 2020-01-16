#include "math4.h"

#include <stdint.h>

float4 float4_Make(float x, float y, float z, float w)
{
	return (float4) {{ x, y, z, w }};
}

float4 float4_Multiply(float4 v, float4x4 m)
{
	//float4 out;
	//for (uint32_t i=0; i<4; ++i) {
	//	float element = 0.0f;
	//	for (uint32_t j=0; j<4; ++j) {
	//		element += v.v[j] * m.m[j][i];
	//	}
	//	out.v[i] = element;
	//}
	//return out;
	const float x = v.v[0] * m.m[0][0] + v.v[1] * m.m[0][1] + v.v[2] * m.m[0][2] + v.v[3] * m.m[0][3];
	const float y = v.v[0] * m.m[1][0] + v.v[1] * m.m[1][1] + v.v[2] * m.m[1][2] + v.v[3] * m.m[1][3];
	const float z = v.v[0] * m.m[2][0] + v.v[1] * m.m[2][1] + v.v[2] * m.m[2][2] + v.v[3] * m.m[2][3];
	const float w = v.v[0] * m.m[3][0] + v.v[1] * m.m[3][1] + v.v[2] * m.m[3][2] + v.v[3] * m.m[3][3];
	return (float4) {{ x, y, z, w }};
}

float4x4 float4x4_Identity()
{
	return (float4x4) {{
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 }
	}};
}

float4x4 float4x4_NonUniformScale(float x, float y, float z)
{
	return (float4x4) {{
		{ x, 0, 0, 0 }, 
		{ 0, y, 0, 0 },
		{ 0, 0, z, 0 },
		{ 0, 0, 0, 1 }
	}};
}

float4x4 float4x4_Translate(float x, float y, float z)
{
	return (float4x4) {{
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ x, y, z, 1 }
	}};
}

float4x4 float4x4_Transpose(float4x4 m)
{
	float4x4 result;
	for (uint32_t i=0; i<4; ++i) {
		for (uint32_t j=0; j<4; ++j) {
			result.m[i][j] = m.m[j][i];
		}
	}
	return result;
}

float4x4 float4x4_Multiply(float4x4 left, float4x4 right)
{
	float4x4 result;
	for (uint32_t i=0; i<4; ++i) {
		for (uint32_t j=0; j<4; ++j) {
			float element = 0.0f;
			for (uint32_t k=0; k<4; ++k) {
				// (i,j) is the element being written
				// Keep row constant for left, column constant for right.
				element += (left.m[i][k] * right.m[k][j]);
			}
			result.m[i][j] = element;
		}
	}
	return result;
}

void float4_DebugPrint(FILE* stream, float4 v)
{
	fprintf(stream, "(%6.5f %6.5f %6.5f %6.5f)\n", v.x, v.y, v.z, v.w);
}

void float4x4_DebugPrint(FILE* stream, float4x4 m)
{
	for (uint32_t i=0; i<4; ++i) {
		fprintf(stream, "%6.5f %6.5f %6.5f %6.5f\n", m.m[i][0], m.m[i][1], m.m[i][2], m.m[i][3]);
	}
}
