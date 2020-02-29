#include <knell/test.h>

#include <knell/kn.h>
#include <knell/binary.h>

KN_TEST_SUITE_BEGIN("binary")
	KN_TEST_UNIT("0 is a power of two?") {
		KN_TEST_ASSERT_TRUE(isPowerOfTwo(0));
	}

	KN_TEST_UNIT("1 is a power of two") {
		KN_TEST_ASSERT_TRUE(isPowerOfTwo(1));
	}

	KN_TEST_UNIT("General cases of power of two") {
		for (uint32_t i = 1; i < 31; ++i) {
			KN_TEST_ASSERT_TRUE(isPowerOfTwo((uint32_t)1 << i));
		}
	}

	KN_TEST_UNIT("Non powers of two") {
		uint32_t nonPowersOfTwo[] = {
			3, 5, 7, 11, 17, 101, 65443, 65422, 4000000
		};
		const size_t valuesToTest = sizeof(nonPowersOfTwo) / sizeof(nonPowersOfTwo[0]);
		for (uint32_t i = 0; i < valuesToTest; ++i) {
			KN_TEST_ASSERT_FALSE(isPowerOfTwo(nonPowersOfTwo[i]));
		}
	}

KN_TEST_SUITE_END
