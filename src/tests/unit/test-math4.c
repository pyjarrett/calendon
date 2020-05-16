#include <knell/test.h>

#include <knell/kn.h>
#include <knell/float.h>

#include <knell/math4.h>

KN_TEST_SUITE_BEGIN("math4")
KN_TEST_UNIT("Vector add") {
	const float4 origin = float4_Make(0.0f, 0.0f, 0.0f, 1.0f);
	const float4x4 translation = float4x4_Translate(1.0f, 2.0f, 3.0f);
	const float4 actual = float4_Multiply(origin, translation);

	const float4 expected = float4_Make(1.0f, 2.0f, 3.0f, 1.0f);
	KN_TEST_ASSERT_CLOSE_F(float4_Distance(expected, actual), 0.0f, 0.5);
}
KN_TEST_SUITE_END
