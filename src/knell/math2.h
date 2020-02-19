/*
 * Two-dimension mathematics primitives.  This includes both dimensions,
 * vectors, planar angles, and transforms for manipulating these elements.
 */
#ifndef KN_MATH2_H
#define KN_MATH2_H

#include "kn.h"

typedef struct {
	float width, height;
} dimension2f;

typedef struct {
	uint32_t width, height;
} dimension2u32;

typedef struct {
	union {
		struct {
			float x, y;
		};
		float v[2];
	};
} float2;

/**
 * An angle in 2D, named to differentiate it from a solid angle (3D).
 *
 * Getting degrees/radians swapped leads to all sorts of bugs, so define a type
 * here and use interfaces which rely on that type.
 */
typedef struct {
	float degrees;
} PlanarAngle;

KN_API float Angle_ToDegrees(PlanarAngle a);
KN_API float Angle_ToRadians(PlanarAngle a);
KN_API PlanarAngle Angle_InDegrees(float d);
KN_API PlanarAngle Angle_InRadians(float d);
KN_API PlanarAngle Angle_Add(PlanarAngle left, PlanarAngle right);
KN_API PlanarAngle Angle_Sub(PlanarAngle left, PlanarAngle right);

KN_API float2 float2_Make(float x, float y);
KN_API float2 float2_FromPolar(float radius, PlanarAngle theta);
KN_API float2 float2_Add(float2 left, float2 right);
KN_API float2 float2_Sub(float2 left, float2 right);
KN_API float2 float2_Multiply(float2 v, float s);
KN_API float2 float2_Divide(float2 v, float s);
KN_API float float2_Length(float2 v);
KN_API float2 float2_Midpoint(float2 left, float2 right);

KN_API float2 float2_Lerp(float2 from, float2 to, float alpha);

KN_API PlanarAngle float2_DirectionBetween(float2 from, float2 to);

#endif /* KN_MATH2_H */
