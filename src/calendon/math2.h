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

/**
 * A row-major matrix for transforming 2D points and vectors.
 */
typedef union {
	float m[3][3];
} CnTransform2;

/**
 * An axis-aligned bounding box in 2D.
 */
typedef struct {
	CnFloat2 min, max;
} CnAABB2;

CN_STATIC_ASSERT(sizeof(CnTransform2) == 9 * sizeof(float), "Padding present in CnTransform2");

CN_API CnPlanarAngle cnPlanarAngle_MakeDegrees(float d);
CN_API CnPlanarAngle cnPlanarAngle_MakeRadians(float r);
CN_API float         cnPlanarAngle_Degrees(CnPlanarAngle a);
CN_API float         cnPlanarAngle_Radians(CnPlanarAngle a);
CN_API CnPlanarAngle cnPlanarAngle_Add(CnPlanarAngle left, CnPlanarAngle right);
CN_API CnPlanarAngle cnPlanarAngle_Sub(CnPlanarAngle left, CnPlanarAngle right);

CN_API CnFloat2      cnFloat2_Make(float x, float y);
CN_API CnFloat2      cnFloat2_FromPolar(float radius, CnPlanarAngle theta);

CN_API CnFloat2      cnFloat2_Add(CnFloat2 left, CnFloat2 right);
CN_API CnFloat2      cnFloat2_Sub(CnFloat2 left, CnFloat2 right);
CN_API CnFloat2      cnFloat2_Multiply(CnFloat2 v, float s);
CN_API CnFloat2      cnFloat2_Divide(CnFloat2 v, float s);

CN_API float         cnFloat2_Length(CnFloat2 v);
CN_API float         cnFloat2_LengthSquared(CnFloat2 v);
CN_API CnFloat2      cnFloat2_Normalize(CnFloat2 v);

CN_API CnFloat2      cnFloat2_Midpoint(CnFloat2 left, CnFloat2 right);
CN_API float         cnFloat2_DistanceSquared(CnFloat2 left, CnFloat2 right);

CN_API CnFloat2      cnFloat2_Lerp(CnFloat2 from, CnFloat2 to, float alpha);

CN_API CnPlanarAngle cnFloat2_DirectionBetween(CnFloat2 from, CnFloat2 to);

CN_API CnTransform2  cnTransform2_MakeIdentity(void);
CN_API CnTransform2  cnTransform2_MakeTranslateXY(float x, float y);
CN_API CnTransform2  cnTransform2_MakeUniformScale(float scale);
CN_API CnTransform2  cnTransform2_MakeRotation(CnPlanarAngle angle);
CN_API CnTransform2  cnTransform2_Combine(CnTransform2 first, CnTransform2 second);
CN_API CnFloat2      cnTransform2_Translation(CnTransform2 transform);
CN_API CnFloat2      cnTransform2_Scale(CnTransform2 transform);

CN_API CnAABB2   cnAABB2_MakeMinMax(CnFloat2 min, CnFloat2 max);
CN_API CnFloat2  cnAABB2_Center(CnAABB2 aabb);
CN_API float     cnAABB2_Width(CnAABB2 aabb);
CN_API float     cnAABB2_Height(CnAABB2 aabb);
CN_API CnAABB2   cnAABB2_IncludePoint(CnAABB2 aabb, CnFloat2 point);
CN_API void      cnAABB2_Corners(CnAABB2 aabb, CnFloat2 point[4]);
CN_API bool      cnAABB2_FullyContainsAABB2(CnAABB2 a, CnAABB2 b, float tolerance);

CN_API CnFloat2        cnMath2_TransformPoint(CnFloat2 point, CnTransform2 transform);
CN_API CnFloat2        cnMath2_TransformVector(CnFloat2 point, CnTransform2 transform);
CN_API CnDimension2u32 cnMath2_TransformDimension2u32(CnDimension2u32 dimension, CnTransform2 transform);
CN_API CnAABB2         cnMath2_TransformAABB2(CnAABB2, CnTransform2 transform);

#ifdef __cplusplus
}
#endif

#endif /* CN_MATH2_H */
