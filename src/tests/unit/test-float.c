#include <knell/test.h>

#include <knell/kn.h>
#include <knell/float.h>

#include <math.h>

KN_TEST_SUITE_BEGIN("float")
	KN_TEST_UNIT("RelativeDiff") {
		KN_TEST_ASSERT_TRUE(fabsf(float_RelativeDiff(1.0f, 2.0f) - 0.5f) < 0.001f);
		KN_TEST_ASSERT_CLOSE_F(float_RelativeDiff(1.0f, 2.0f), 0.5f, 0.001f);

		KN_TEST_ASSERT_TRUE(fabsf(float_RelativeDiff(1.0f, 4.0f) - 0.75f) < 0.001f);
		KN_TEST_ASSERT_CLOSE_F(float_RelativeDiff(1.0f, 4.0f), 0.75f, 0.001f);

		KN_TEST_ASSERT_TRUE(fabsf(float_RelativeDiff(1.0f, 10.0f) - 0.90) < 0.001f);
		KN_TEST_ASSERT_CLOSE_F(float_RelativeDiff(1.0f, 10.0f), 0.90f, 0.001f);

		KN_TEST_ASSERT_TRUE(fabsf(float_RelativeDiff(1.0f, 100.0f) - 0.99) < 0.001f);
		KN_TEST_ASSERT_CLOSE_F(float_RelativeDiff(1.0f, 100.0f), 0.99f, 0.001f);
	}

	KN_TEST_UNIT("RelativeDiff of zeros") {
		KN_TEST_ASSERT_TRUE(fabsf(float_RelativeDiff(0.0f, 0.0f) - 0.0f) < 0.001f);
	}
KN_TEST_SUITE_END
