#include <calendon/test.h>

#include <calendon/cn.h>
#include <calendon/binary.h>

CN_TEST_SUITE_BEGIN("binary")
	CN_TEST_UNIT("0 is a power of two?") {
		CN_TEST_ASSERT_TRUE(cnIsPowerOfTwo(0));
	}

	CN_TEST_UNIT("1 is a power of two") {
		CN_TEST_ASSERT_TRUE(cnIsPowerOfTwo(1));
	}

	CN_TEST_UNIT("General cases of power of two") {
		for (uint32_t i = 1; i < 31; ++i) {
			CN_TEST_ASSERT_TRUE(cnIsPowerOfTwo((uint32_t) 1 << i));
		}
	}

	CN_TEST_UNIT("Non powers of two") {
		uint32_t nonPowersOfTwo[] = {
			3, 5, 7, 11, 17, 101, 65443, 65422, 4000000
		};
		const size_t valuesToTest = sizeof(nonPowersOfTwo) / sizeof(nonPowersOfTwo[0]);
		for (uint32_t i = 0; i < valuesToTest; ++i) {
			CN_TEST_ASSERT_FALSE(cnIsPowerOfTwo(nonPowersOfTwo[i]));
		}
	}

CN_TEST_SUITE_END
