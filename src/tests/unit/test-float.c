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

	CN_TEST_UNIT("Clamp: float to [0, 1].") {
		// Test far outside the boundaries.
		CN_TEST_ASSERT_EXACT_F(cnFloat_Clamp(-10.0f, 0.0f, 1.0f), 0.0f);
		CN_TEST_ASSERT_EXACT_F(cnFloat_Clamp(10.0f, 0.0f, 1.0f), 1.0f);

		// Check boundary conditions.
		CN_TEST_ASSERT_EXACT_F(cnFloat_Clamp(0.0f, 0.0f, 1.0f), 0.0f);
		CN_TEST_ASSERT_EXACT_F(cnFloat_Clamp(1.0f, 0.0f, 1.0f), 1.0f);

		// Check within the boundary.
		CN_TEST_ASSERT_EXACT_F(cnFloat_Clamp(0.2f, 0.0f, 1.0f), 0.2f);
		CN_TEST_ASSERT_EXACT_F(cnFloat_Clamp(0.2f, 1e-15f, 1e20f), 0.2f);
	}

	CN_TEST_UNIT("Clamp: float to negative bounds.") {
		// Test far outside the boundaries.
		CN_TEST_ASSERT_EXACT_F(cnFloat_Clamp(-10.0f, -20.0f, -1.0f), -10.0f);
		CN_TEST_ASSERT_EXACT_F(cnFloat_Clamp(10.0f, -20.0f, -1.0f), -1.0f);

		// Check boundary conditions.
		CN_TEST_ASSERT_EXACT_F(cnFloat_Clamp(-20.0f, -20.0f, -1.0f), -20.0f);
		CN_TEST_ASSERT_EXACT_F(cnFloat_Clamp(-1.0f, -20.0f, -1.0f), -1.0f);

		// Check within the boundary.
		CN_TEST_ASSERT_EXACT_F(cnFloat_Clamp(-10.0f, -20.0f, -1.0f), -10.0f);
		CN_TEST_ASSERT_EXACT_F(cnFloat_Clamp(-1.0f, -1e14f, -1e-10f), -1.0f);
	}

	CN_TEST_UNIT("Clamp: Precondition checks on infinity.") {
		CN_TEST_PRECONDITION(cnFloat_Clamp(INFINITY, 0.0f, 1.0f));
		CN_TEST_PRECONDITION(cnFloat_Clamp(-INFINITY, 0.0f, 1.0f));
	}

	CN_TEST_UNIT("Clamp: Precondition checks on NaN.") {
		CN_TEST_PRECONDITION(cnFloat_Clamp(NAN, 0.0f, 1.0f));
	}
CN_TEST_SUITE_END
