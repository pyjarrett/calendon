#include <stdlib.h>
#include <stdio.h>

#include "test.h"
#include <knell/font-psf2.h>

KN_TEST_SUITE_BEGIN("Frame allocator tests")

	KN_TEST_UNIT("Bytes in UTF-8 code point") {
		KN_TEST_ASSERT_EQ_U8(Font_BytesInUtf8CodePoint('a'), 1);
		KN_TEST_ASSERT_EQ_U8(Font_BytesInUtf8CodePoint('\xC2'), 2);
		KN_TEST_ASSERT_EQ_U8(Font_BytesInUtf8CodePoint('\xE2'), 3);
	}

	KN_TEST_UNIT("Code points match happy case") {
		KN_TEST_ASSERT_TRUE(Font_Utf8CodePointsMatch("b", "b"));
		KN_TEST_ASSERT_TRUE(Font_Utf8CodePointsMatch("\xEF\xBF\xBE", "\xEF\xBF\xBE"));
	}

	KN_TEST_UNIT("Code points don't match") {
		KN_TEST_ASSERT_FALSE(Font_Utf8CodePointsMatch("a", "b"));
		KN_TEST_ASSERT_FALSE(Font_Utf8CodePointsMatch("\xEF\xBF\xBE", "\xEF\xBF"));
		KN_TEST_ASSERT_FALSE(Font_Utf8CodePointsMatch("\xEF\xBF\xBE", "\xEF\0\xBF"));
	}

KN_TEST_SUITE_END
