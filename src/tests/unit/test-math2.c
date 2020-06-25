#include <calendon/test.h>

#include <calendon/cn.h>
#include <calendon/float.h>

#include <calendon/math2.h>

CN_TEST_SUITE_BEGIN("math2")
	CN_TEST_UNIT("Identity") {
		const CnFloat2 float2 = cnFloat2_Make(3.0f, 5.0f);
		const CnTransform2 identity = cnTransform2_MakeIdentity();
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnMath2_TransformPoint(float2, identity), float2) < 0.1f);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnMath2_TransformVector(float2, identity), float2) < 0.1f);
	}

	CN_TEST_UNIT("Point translate") {
		const CnFloat2 point = cnFloat2_Make(3.0f, 5.0f);
		const CnTransform2 translate = cnTransform2_MakeTranslateXY(7.0f, 11.0f);
		const CnFloat2 expected = cnFloat2_Make(10.0f, 16.0f);
		const CnFloat2 actual = cnMath2_TransformPoint(point, translate);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(actual, expected) < 0.1f);
	}

	CN_TEST_UNIT("Vector translate") {
		const CnFloat2 vector = cnFloat2_Make(3.0f, 5.0f);
		const CnTransform2 translate = cnTransform2_MakeTranslateXY(7.0f, 11.0f);
		const CnFloat2 expected = cnFloat2_Make(3.0f, 5.0f);
		const CnFloat2 actual = cnMath2_TransformVector(vector, translate);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(actual, expected) < 0.1f);
	}

	CN_TEST_UNIT("Point scale") {
		const CnFloat2 point = cnFloat2_Make(3.0f, 5.0f);
		const CnTransform2 scale = cnTransform2_MakeUniformScale(2.0f);
		const CnFloat2 expected = cnFloat2_Make(6.0f, 10.0f);
		const CnFloat2 actual = cnMath2_TransformPoint(point, scale);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(actual, expected) < 0.1f);
	}

	CN_TEST_UNIT("Vector scale") {
		const CnFloat2 vector = cnFloat2_Make(3.0f, 5.0f);
		const CnTransform2 scale = cnTransform2_MakeUniformScale(2.0f);
		const CnFloat2 expected = cnFloat2_Make(6.0f, 10.0f);
		const CnFloat2 actual = cnMath2_TransformVector(vector, scale);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(actual, expected) < 0.1f);
	}

	CN_TEST_UNIT("Rotation") {
		const CnFloat2 float2 = cnFloat2_Make(1.0f, 0.0f);
		{
			const CnTransform2 ccw45 = cnTransform2_MakeRotation(cnPlanarAngle_MakeDegrees(45));
			const CnFloat2 expected = cnFloat2_Make(0.7071f, 0.7071f);
			CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
				cnMath2_TransformPoint(float2, ccw45), expected) < 0.1f);
			CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
				cnMath2_TransformVector(float2, ccw45), expected) < 0.1f);
		}

		{
			const CnTransform2 cw45 = cnTransform2_MakeRotation(cnPlanarAngle_MakeDegrees(-45));
			const CnFloat2 expected = cnFloat2_Make(0.7071f, -0.7071f);
			CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
				cnMath2_TransformPoint(float2, cw45), expected) < 0.1f);
			CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
				cnMath2_TransformVector(float2, cw45), expected) < 0.1f);
		}

		{
			const CnTransform2 ccw135 = cnTransform2_MakeRotation(cnPlanarAngle_MakeDegrees(135));
			const CnFloat2 expected = cnFloat2_Make(-0.7071f, 0.7071f);
			CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
				cnMath2_TransformPoint(float2, ccw135), expected) < 0.1f);
			CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
				cnMath2_TransformVector(float2, ccw135), expected) < 0.1f);
		}

		{
			const CnTransform2 ccw315 = cnTransform2_MakeRotation(cnPlanarAngle_MakeDegrees(315));
			const CnFloat2 expected = cnFloat2_Make(0.7071f, -0.7071f);
			CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
				cnMath2_TransformPoint(float2, ccw315), expected) < 0.1f);
			CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
				cnMath2_TransformVector(float2, ccw315), expected) < 0.1f);
		}
	}
CN_TEST_SUITE_END
