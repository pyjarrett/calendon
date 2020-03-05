#include "math2.h"

#include "kn.h"
#include <math.h>

static const float pi = 3.14159265f;

KN_API float Angle_ToDegrees(PlanarAngle a)
{
	return a.degrees;
}

KN_API float Angle_ToRadians(PlanarAngle a)
{
	return a.degrees * pi / 180.0f;
}

KN_API PlanarAngle Angle_InDegrees(float d)
{
	return (PlanarAngle) { d };
}

KN_API PlanarAngle Angle_InRadians(float r)
{
	return (PlanarAngle) { r * 180.0f / pi };
}

KN_API PlanarAngle Angle_Add(PlanarAngle left, PlanarAngle right)
{
	return (PlanarAngle) { left.degrees + right.degrees };
}

KN_API PlanarAngle Angle_Sub(PlanarAngle left, PlanarAngle right)
{
	return (PlanarAngle) { left.degrees - right.degrees };
}

KN_API float2 float2_Make(float x, float y)
{
	return (float2) { x, y };
}

KN_API float2 float2_FromPolar(float radius, PlanarAngle theta)
{
	return (float2) { radius * cosf(Angle_ToRadians(theta)),
		radius * sinf(Angle_ToRadians(theta)) };
}

KN_API float2 float2_Add(float2 left, float2 right)
{
	return (float2) { left.x + right.x, left.y + right.y };
}

KN_API float2 float2_Sub(float2 left, float2 right)
{
	return (float2) { left.x - right.x, left.y - right.y };
}

KN_API float2 float2_Multiply(float2 v, float s)
{
	return (float2) { v.x * s, v.y * s };
}

KN_API float2 float2_Divide(float2 v, float s)
{
	return (float2) { v.x / s, v.y / s };
}

KN_API float float2_Length(float2 v)
{
	return sqrtf(v.x * v.x + v.y * v.y);
}

KN_API float float2_LengthSquared(float2 v)
{
	return v.x * v.x + v.y * v.y;
}

KN_API float2 float2_Midpoint(float2 left, float2 right)
{
	return float2_Divide(float2_Add(left, right), 2.0f);
}

KN_API float float2_DistanceSquared(float2 left, float2 right)
{
	return float2_LengthSquared(float2_Make(left.x - right.x, left.y - right.y));
}

KN_API float2 float2_Lerp(float2 from, float2 to, float alpha)
{
	KN_ASSERT(0.0f <= alpha && alpha <= 1.0f, "Alpha %f is not in range [0,1]", alpha);
	return float2_Add(float2_Multiply(from, 1.0f - alpha), float2_Multiply(to, alpha));
}

KN_API PlanarAngle float2_DirectionBetween(float2 from, float2 to)
{
	return Angle_InRadians(atan2f(to.y - from.y, to.x - from.x));
}
