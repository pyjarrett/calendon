/*
 * Two-dimension mathematics primitives.  This includes both dimensions, vectors,
 * and transforms for manipulating these elements.
 */
#ifndef KN_MATH2_H
#define KN_MATH2_H

#include "kn.h"

typedef struct {
	float x, y;
} float2;

KN_API float2 float2_Make(float x, float y);

#endif /* KN_MATH2_H */
