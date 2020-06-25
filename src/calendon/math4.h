/*
 * Mathematical types for rendering and collision detection.
 *
 * This assumes the use of row vectors.  This means the
 * correct way to multiply a matrix M and a vector v is (v * M).
 */
#ifndef CN_MATH4_H
#define CN_MATH4_H

#include <calendon/cn.h>

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
} CnFloat4;

/**
 * Using float[4][4] gives the row-major ordering of matrix data:
 *  0  1  2  3
 *  4  5  6  7
 *  8  9 10 11
 * 12 13 14 15
 */
typedef struct {
	float m[4][4];
} CnFloat4x4;

CN_TEST_API CnFloat4 cnFloat4_Make(float x, float y, float z, float w);
CN_TEST_API CnFloat4 cnFloat4_Multiply(CnFloat4 v, CnFloat4x4 m);
CN_TEST_API float cnFloat4_Distance(CnFloat4 a, CnFloat4 b);

CN_TEST_API CnFloat4x4 cnFloat4x4_Make(float* values);
CN_TEST_API CnFloat4x4 cnFloat4x4_Identity(void);
CN_TEST_API CnFloat4x4 cnFloat4x4_NonUniformScale(float x, float y, float z);
CN_TEST_API CnFloat4x4 cnFloat4x4_Translate(float x, float y, float z);
CN_TEST_API CnFloat4x4 cnFloat4x4_Transpose(CnFloat4x4 m);
CN_TEST_API CnFloat4x4 cnFloat4x4_Multiply(CnFloat4x4 left, CnFloat4x4 right);

#include <stdio.h>
CN_TEST_API void cnFloat4_DebugPrint(FILE* stream, CnFloat4 v);
CN_TEST_API void cnFloat4x4_DebugPrint(FILE* stream, CnFloat4x4 m);

#ifdef __cplusplus
}
#endif

#endif /* CN_MATH_H */
