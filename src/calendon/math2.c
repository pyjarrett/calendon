#include "math2.h"

#include "cn.h"
#include <math.h>

static const float pi = 3.14159265f;

float cnPlanarAngle_Degrees(CnPlanarAngle a)
{
	return a.degrees;
}

float cnPlanarAngle_Radians(CnPlanarAngle a)
{
	return a.degrees * pi / 180.0f;
}

CnPlanarAngle cnPlanarAngle_MakeDegrees(float d)
{
	return (CnPlanarAngle) { d };
}

CnPlanarAngle cnPlanarAngle_MakeRadians(float r)
{
	return (CnPlanarAngle) { r * 180.0f / pi };
}

CnPlanarAngle cnPlanarAngle_Add(CnPlanarAngle left, CnPlanarAngle right)
{
	return (CnPlanarAngle) { left.degrees + right.degrees };
}

CnPlanarAngle cnPlanarAngle_Sub(CnPlanarAngle left, CnPlanarAngle right)
{
	return (CnPlanarAngle) { left.degrees - right.degrees };
}

CnFloat2 cnFloat2_Make(float x, float y)
{
	return (CnFloat2) { x, y };
}

CnFloat2 cnFloat2_FromPolar(float radius, CnPlanarAngle theta)
{
	return (CnFloat2) { radius * cosf(cnPlanarAngle_Radians(theta)),
		radius * sinf(cnPlanarAngle_Radians(theta)) };
}

CnFloat2 cnFloat2_Add(CnFloat2 left, CnFloat2 right)
{
	return (CnFloat2) { left.x + right.x, left.y + right.y };
}

CnFloat2 cnFloat2_Sub(CnFloat2 left, CnFloat2 right)
{
	return (CnFloat2) { left.x - right.x, left.y - right.y };
}

CnFloat2 cnFloat2_Multiply(CnFloat2 v, float s)
{
	return (CnFloat2) { v.x * s, v.y * s };
}

CnFloat2 cnFloat2_Divide(CnFloat2 v, float s)
{
	return (CnFloat2) { v.x / s, v.y / s };
}

float cnFloat2_Length(CnFloat2 v)
{
	return sqrtf(v.x * v.x + v.y * v.y);
}

float cnFloat2_LengthSquared(CnFloat2 v)
{
	return v.x * v.x + v.y * v.y;
}

CnFloat2 cnFloat2_Normalize(CnFloat2 v)
{
	const float length = cnFloat2_Length(v);
	CN_ASSERT(length > 0.0f, "Cannot normalize a vector of 0 length.");
	return cnFloat2_Divide(v, length);
}

CnFloat2 cnFloat2_Midpoint(CnFloat2 left, CnFloat2 right)
{
	return cnFloat2_Divide(cnFloat2_Add(left, right), 2.0f);
}

float cnFloat2_DistanceSquared(CnFloat2 left, CnFloat2 right)
{
	return cnFloat2_LengthSquared(cnFloat2_Make(left.x - right.x, left.y - right.y));
}

CnFloat2 cnFloat2_Lerp(CnFloat2 from, CnFloat2 to, float alpha)
{
	CN_ASSERT(0.0f <= alpha && alpha <= 1.0f, "Alpha %f is not in range [0,1]", alpha);
	return cnFloat2_Add(cnFloat2_Multiply(from, 1.0f - alpha), cnFloat2_Multiply(to, alpha));
}

CnPlanarAngle cnFloat2_DirectionBetween(CnFloat2 from, CnFloat2 to)
{
	return cnPlanarAngle_MakeRadians(atan2f(to.y - from.y, to.x - from.x));
}

CnTransform2 cnTransform2_MakeIdentity(void)
{
	return (CnTransform2) {{
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f }
	}};
}

CnTransform2 cnTransform2_MakeTranslateXY(float x, float y)
{
	CN_ASSERT_FINITE_F32(x);
	CN_ASSERT_FINITE_F32(y);

	return (CnTransform2) {{
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{    x,    y, 1.0f }
	}};
}

CnTransform2 cnTransform2_MakeUniformScale(float scale)
{
	CN_ASSERT_FINITE_F32(scale);
	return (CnTransform2) {{
		{ scale, 0.0f, 0.0f },
		{ 0.0f, scale, 0.0f },
		{ 0.0f, 0.0f, 1.0f }
	}};
}

/**
 * Creates a transform to represent a counter-clockwise rotation.
 */
CnTransform2 cnTransform2_MakeRotation(CnPlanarAngle angle)
{
	const float theta = cnPlanarAngle_Radians(angle);
	CN_ASSERT_FINITE_F32(theta);

	return (CnTransform2) {{
		{  cosf(theta), sinf(theta), 0.0f },
		{ -sinf(theta), cosf(theta), 0.0f },
		{         0.0f,        0.0f, 1.0f }
	}};
}

CnTransform2 cnTransform2_Combine(CnTransform2 first, CnTransform2 second)
{
	CnTransform2 result;
	// Loop over all positions in `result`.
	// TODO: This should be done with SIMD.
	for (uint32_t i = 0; i < 3; ++i) {
		for (uint32_t j = 0; j < 3; ++j) {
			// Multiply to find result[i][j].
			// Use a row from `first`, and a column from `second`.
			result.m[i][j] = 0.0f;
			for (uint32_t k = 0; k < 3; ++k) {
				result.m[i][j] += (first.m[k][j] * second.m[i][k]);
			}
			CN_ASSERT_FINITE_F32(result.m[i][j]);
		}
	}
	return result;
}

/**
 * Extracts the translation from a transform.
 */
CnFloat2 cnTransform2_Translation(CnTransform2 transform)
{
	return cnFloat2_Make(transform.m[2][0], transform.m[2][1]);
}

/**
 * Extracts the scale from a transform.
 */
CnFloat2 cnTransform2_Scale(CnTransform2 transform)
{
	const float x1 = transform.m[0][0];
	const float x2 = transform.m[0][1];
	const float y1 = transform.m[1][0];
	const float y2 = transform.m[1][1];
	return cnFloat2_Make(sqrtf(x1*x1 + x2*x2), sqrtf(y1*y1 + y2*y2));
}

CnAABB2 cnAABB2_MakeMinMax(CnFloat2 min, CnFloat2 max)
{
	CN_ASSERT(min.x <= max.x, "AABB2 invalid bounds: X min: %f > X max: %f", min.x, max.y);
	CN_ASSERT(min.y <= max.y, "AABB2 invalid bounds: Y min: %f > Y max: %f", min.y, max.y);
	return (CnAABB2) { min, max };
}

CnFloat2 cnAABB2_Center(CnAABB2 aabb)
{
	return cnFloat2_Midpoint(aabb.min, aabb.max);
}

float cnAABB2_Width(CnAABB2 aabb)
{
	return aabb.max.x - aabb.min.x;
}

float cnAABB2_Height(CnAABB2 aabb)
{
	return aabb.max.y - aabb.min.y;
}

/**
 * Creates an expanded AABB to include the given point.
 */
CnAABB2 cnAABB2_IncludePoint(CnAABB2 aabb, CnFloat2 point)
{
	return cnAABB2_MakeMinMax(
		cnFloat2_Make(fminf(aabb.min.x, point.x), fminf(aabb.min.y, point.y)),
		cnFloat2_Make(fmaxf(aabb.max.x, point.x), fmaxf(aabb.max.y, point.y))
	);
}

void cnAABB2_Corners(CnAABB2 aabb, CnFloat2 point[4])
{
	point[0] = aabb.min;
	point[1] = cnFloat2_Make(aabb.max.x, aabb.min.y);
	point[2] = cnFloat2_Make(aabb.min.x, aabb.max.y);
	point[3] = aabb.max;
}

bool cnAABB2_FullyContainsAABB2(CnAABB2 area, CnAABB2 object, float tolerance)
{
	CN_ASSERT_FINITE_F32(tolerance);

	return area.min.x <= object.min.x + tolerance
		&& area.min.y <= object.min.y + tolerance
		&& area.max.x >= object.max.x - tolerance
		&& area.max.y >= object.max.y - tolerance;
}

/**
 * Transforming a point applies the rotation, scale and translation of the
 * transform.
 */
CnFloat2 cnMath2_TransformPoint(CnFloat2 point, CnTransform2 transform)
{
	return cnFloat2_Make(
		point.x * transform.m[0][0] + point.y * transform.m[1][0] + 1.0f * transform.m[2][0],
		point.x * transform.m[0][1] + point.y * transform.m[1][1] + 1.0f * transform.m[2][1]
	);
}

/**
 * Transforming a vector can rotate an scale the vector.  Vectors do not have
 * a position, so they cannot be translated.
 */
CnFloat2 cnMath2_TransformVector(CnFloat2 point, CnTransform2 transform)
{
	return cnFloat2_Make(
		point.x * transform.m[0][0] + point.y * transform.m[1][0],
		point.x * transform.m[0][1] + point.y * transform.m[1][1]
	);
}

/**
 * Applies the scaling factors of a transform to a dimenion.
 */
CnDimension2u32 cnMath2_TransformDimension2u32(CnDimension2u32 dimension,
	CnTransform2 transform)
{
	const CnFloat2 scale = cnTransform2_Scale(transform);
	CN_ASSERT_FINITE_F32(scale.x);
	CN_ASSERT_FINITE_F32(scale.y);

	return (CnDimension2u32) {
		(uint32_t)roundf(dimension.width * scale.x),
		(uint32_t)roundf(dimension.height * scale.y) };
}

/**
 * Applies a transformation to the four corners of an AABB, and then generate
 * a new AABB from those points.
 *
 * Note that applying a rotation to an AABB will result in a AABB which contains
 * the rotated corners of the original AABB, since an AABB by definition always
 * has two sides parallel and two sides perpendicular to each axis.
 */
CnAABB2 cnMath2_TransformAABB2(CnAABB2 aabb, CnTransform2 transform)
{
	CnFloat2 points[4];
	cnAABB2_Corners(aabb, points);

	for (uint32_t i = 0; i < 4; ++i) {
		points[i] = cnMath2_TransformPoint(points[i], transform);
	}

	CnAABB2 result = cnAABB2_MakeMinMax(points[0], points[0]);
	result = cnAABB2_IncludePoint(result, points[1]);
	result = cnAABB2_IncludePoint(result, points[2]);
	return cnAABB2_IncludePoint(result, points[3]);
}
