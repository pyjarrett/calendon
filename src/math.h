/*
 * Mathematical types for rendering and collision detection.
 *
 * This mathematics library assumes the use of column vectors.  This means the
 * correct way to multiply a matrix M and a vector v is (M * v).
 */
#ifndef KN_MATH_H
#define KN_MATH_H

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
 *
 * Using float[16] gives the column-major ordering of matrix data:
 *  0  4  8 12
 *  1  5  9 13
 *  2  6 10 14
 *  3  3 11 15

 * This means that matrices can be written in the form of a singular array:
 * ```
 * float translate[16] = { 1, 0, 0, x,
 *                         0, 1, 0, y,
 *                         0, 0, 1, z,
 *                         0, 0, 0, 1 };
 * ```
 */
typedef struct {
	float m[4][4];
} float4x4;

float4 float4_Make(float x, float y, float z, float w);
float4 float4_multiply(float4 v, float4x4 m);

float4x4 float4x4_identity();
float4x4 float4x4_nonuniformScale(float x, float y, float z);
float4x4 float4x4_translate(float x, float y, float z);
float4x4 float4x4_transpose(float4x4 m);
float4x4 float4x4_multiply(float4x4 left, float4x4 right);

#include <stdio.h>
void float4_debugPrint(FILE* stream, float4 v);
void float4x4_debugPrint(FILE* stream, float4x4 m);

#endif /* KN_MATH_H */

