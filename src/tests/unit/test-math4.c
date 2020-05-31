#include <calendon/test.h>

#include <calendon/cn.h>
#include <calendon/float.h>

#include <calendon/math4.h>

CN_TEST_SUITE_BEGIN("math4")
	CN_TEST_UNIT("Vector add") {
		const CnFloat4 origin = cnFloat4_Make(0.0f, 0.0f, 0.0f, 1.0f);
		const CnFloat4x4 translation = cnFloat4x4_Translate(1.0f, 2.0f, 3.0f);
		const CnFloat4 actual = cnFloat4_Multiply(origin, translation);

		const CnFloat4 expected = cnFloat4_Make(1.0f, 2.0f, 3.0f, 1.0f);
		CN_TEST_ASSERT_CLOSE_F(cnFloat4_Distance(expected, actual), 0.0f, 0.5);
	}
CN_TEST_SUITE_END
