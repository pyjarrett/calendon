/*
 * Two-dimension mathematics primitives.  This includes both dimensions,
 * vectors, planar angles, and transforms for manipulating these elements.
 */
#ifndef CN_MATH2_H
#define CN_MATH2_H

#include "cn.h"

#include <calendon/dimension.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	union {
		struct {
			float x, y;
		};
		float v[2];
	};
} CnFloat2;

/**
 * An angle in 2D, named to differentiate it from a solid angle (3D).
 *
 * Getting degrees/radians swapped leads to all sorts of bugs, so define a type
 * here and use interfaces which rely on that type.
 */
typedef struct {
	float degrees;
} CnPlanarAngle;

CN_API float cnAngle_ToDegrees(CnPlanarAngle a);
CN_API float cnAngle_ToRadians(CnPlanarAngle a);
CN_API CnPlanarAngle cnAngle_InDegrees(float d);
CN_API CnPlanarAngle cnAngle_InRadians(float d);
CN_API CnPlanarAngle cnAngle_Add(CnPlanarAngle left, CnPlanarAngle right);
CN_API CnPlanarAngle cnAngle_Sub(CnPlanarAngle left, CnPlanarAngle right);

CN_API CnFloat2 cnFloat2_Make(float x, float y);
CN_API CnFloat2 cnFloat2_FromPolar(float radius, CnPlanarAngle theta);
CN_API CnFloat2 cnFloat2_Add(CnFloat2 left, CnFloat2 right);
CN_API CnFloat2 cnFloat2_Sub(CnFloat2 left, CnFloat2 right);
CN_API CnFloat2 cnFloat2_Multiply(CnFloat2 v, float s);
CN_API CnFloat2 cnFloat2_Divide(CnFloat2 v, float s);
CN_API float cnFloat2_Length(CnFloat2 v);
CN_API float cnFloat2_LengthSquared(CnFloat2 v);
CN_API CnFloat2 cnFloat2_Normalize(CnFloat2 v);
CN_API CnFloat2 cnFloat2_Midpoint(CnFloat2 left, CnFloat2 right);
CN_API float cnFloat2_DistanceSquared(CnFloat2 left, CnFloat2 right);

CN_API CnFloat2 cnFloat2_Lerp(CnFloat2 from, CnFloat2 to, float alpha);

CN_API CnPlanarAngle cnFloat2_DirectionBetween(CnFloat2 from, CnFloat2 to);

#ifdef __cplusplus
}
#endif

#endif /* CN_MATH2_H */
