#include <calendon/test.h>

#include <calendon/cn.h>
#include <calendon/float.h>

#include <math.h>

CN_TEST_SUITE_BEGIN("float")
	CN_TEST_UNIT("RelativeDiff") {
		CN_TEST_ASSERT_TRUE(fabsf(cnFloat_RelativeDiff(1.0f, 2.0f) - 0.5f) < 0.001f);
		CN_TEST_ASSERT_CLOSE_F(cnFloat_RelativeDiff(1.0f, 2.0f), 0.5f, 0.001f);

		CN_TEST_ASSERT_TRUE(fabsf(cnFloat_RelativeDiff(1.0f, 4.0f) - 0.75f) < 0.001f);
		CN_TEST_ASSERT_CLOSE_F(cnFloat_RelativeDiff(1.0f, 4.0f), 0.75f, 0.001f);

		CN_TEST_ASSERT_TRUE(fabsf(cnFloat_RelativeDiff(1.0f, 10.0f) - 0.90f) < 0.001f);
		CN_TEST_ASSERT_CLOSE_F(cnFloat_RelativeDiff(1.0f, 10.0f), 0.90f, 0.001f);

		CN_TEST_ASSERT_TRUE(fabsf(cnFloat_RelativeDiff(1.0f, 100.0f) - 0.99f) < 0.001f);
		CN_TEST_ASSERT_CLOSE_F(cnFloat_RelativeDiff(1.0f, 100.0f), 0.99f, 0.001f);
	}

	CN_TEST_UNIT("RelativeDiff of zeros") {
		CN_TEST_ASSERT_TRUE(fabsf(cnFloat_RelativeDiff(0.0f, 0.0f) - 0.0f) < 0.001f);
	}
CN_TEST_SUITE_END
