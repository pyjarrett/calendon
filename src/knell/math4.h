/*
 * Mathematical types for rendering and collision detection.
 *
 * This assumes the use of row vectors.  This means the
 * correct way to multiply a matrix M and a vector v is (v * M).
 */
#ifndef KN_MATH4_H
#define KN_MATH4_H

#include <knell/kn.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ROW vector.
 */
typedef union {
	struct {
		float x, y, z, w;
	};
	float v[4];
} float4;

/**
 * Using float[4][4] gives the row-major ordering of matrix data:
 *  0  1  2  3
 *  4  5  6  7
 *  8  9 10 11
 * 12 13 14 15
 */
typedef struct {
	float m[4][4];
} float4x4;

KN_TEST_API float4 float4_Make(float x, float y, float z, float w);
KN_TEST_API float4 float4_Multiply(float4 v, float4x4 m);
KN_TEST_API float float4_Distance(float4 a, float4 b);

KN_TEST_API float4x4 float4x4_Identity(void);
KN_TEST_API float4x4 float4x4_NonUniformScale(float x, float y, float z);
KN_TEST_API float4x4 float4x4_Translate(float x, float y, float z);
KN_TEST_API float4x4 float4x4_Transpose(float4x4 m);
KN_TEST_API float4x4 float4x4_Multiply(float4x4 left, float4x4 right);

#include <stdio.h>
KN_TEST_API void float4_DebugPrint(FILE* stream, float4 v);
KN_TEST_API void float4x4_DebugPrint(FILE* stream, float4x4 m);

#ifdef __cplusplus
}
#endif

#endif /* KN_MATH_H */
