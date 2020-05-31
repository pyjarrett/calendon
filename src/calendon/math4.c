#include "math4.h"

#include <math.h>

CnFloat4 cnFloat4_Make(float x, float y, float z, float w)
{
	return (CnFloat4) { { x, y, z, w }};
}

CnFloat4 cnFloat4_Multiply(CnFloat4 v, CnFloat4x4 m)
{
	//           [ m[0][0] |    m[0][1] |
	// [x y z w] [ m[1][0] |    m[1][1] |
	//           [ m[2][0] |    m[2][1] |
	//           [ m[3][0] v    m[3][1] v
	//
	const float x = v.v[0] * m.m[0][0] + v.v[1] * m.m[1][0] + v.v[2] * m.m[2][0] + v.v[3] * m.m[3][0];
	const float y = v.v[0] * m.m[0][1] + v.v[1] * m.m[1][1] + v.v[2] * m.m[2][1] + v.v[3] * m.m[3][1];
	const float z = v.v[0] * m.m[0][2] + v.v[1] * m.m[1][2] + v.v[2] * m.m[2][2] + v.v[3] * m.m[3][2];
	const float w = v.v[0] * m.m[0][3] + v.v[1] * m.m[1][3] + v.v[2] * m.m[2][3] + v.v[3] * m.m[3][3];
	return (CnFloat4) { { x, y, z, w }};
}

/**
 * The distance between the x,y and z components of two vectors.
 *
 * Don't consider w to be part of distance, since w is part of the projection.
*/
float cnFloat4_Distance(CnFloat4 a, CnFloat4 b)
{
	const float dx = (a.x - b.x);
	const float dy = (a.y - b.y);
	const float dz = (a.z - b.z);
	return sqrtf(dx*dx + dy*dy + dz*dz);
}

CnFloat4x4 cnFloat4x4_Identity(void)
{
	return (CnFloat4x4) { {
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 }
	}};
}

CnFloat4x4 cnFloat4x4_NonUniformScale(float x, float y, float z)
{
	return (CnFloat4x4) { {
		{ x, 0, 0, 0 }, 
		{ 0, y, 0, 0 },
		{ 0, 0, z, 0 },
		{ 0, 0, 0, 1 }
	}};
}

CnFloat4x4 cnFloat4x4_Translate(float x, float y, float z)
{
	return (CnFloat4x4) { {
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ x, y, z, 1 }
	}};
}

CnFloat4x4 cnFloat4x4_Transpose(CnFloat4x4 m)
{
	CnFloat4x4 result;
	for (uint32_t i=0; i<4; ++i) {
		for (uint32_t j=0; j<4; ++j) {
			result.m[i][j] = m.m[j][i];
		}
	}
	return result;
}

CnFloat4x4 cnFloat4x4_Multiply(CnFloat4x4 left, CnFloat4x4 right)
{
	CnFloat4x4 result;
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

void cnFloat4_DebugPrint(FILE* stream, CnFloat4 v)
{
	fprintf(stream, "(%6.5f %6.5f %6.5f %6.5f)\n", v.x, v.y, v.z, v.w);
}

void cnFloat4x4_DebugPrint(FILE* stream, CnFloat4x4 m)
{
	for (uint32_t i=0; i<4; ++i) {
		fprintf(stream, "%6.5f %6.5f %6.5f %6.5f\n", m.m[i][0], m.m[i][1], m.m[i][2], m.m[i][3]);
	}
}
