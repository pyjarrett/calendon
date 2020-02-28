#include <knell/test.h>

#include <knell/utf8.h>

#include <stdlib.h>
#include <stdio.h>

KN_TEST_SUITE_BEGIN("UTF-8")

	KN_TEST_UNIT("Bytes in UTF-8 code point") {
		KN_TEST_ASSERT_EQ_U8(Utf8_NumBytesInCodePoint('a'), 1);
		KN_TEST_ASSERT_EQ_U8(Utf8_NumBytesInCodePoint('\xC2'), 2);
		KN_TEST_ASSERT_EQ_U8(Utf8_NumBytesInCodePoint('\xE2'), 3);
	}

	KN_TEST_UNIT("Code points match happy case") {
		KN_TEST_ASSERT_TRUE(Utf8_CodePointsMatch("b", "b"));
		KN_TEST_ASSERT_TRUE(Utf8_CodePointsMatch("\xEF\xBF\xBE", "\xEF\xBF\xBE"));
	}

	KN_TEST_UNIT("Code points don't match") {
		KN_TEST_PRECONDITION(Utf8_CodePointsMatch(NULL, "b"));
		KN_TEST_PRECONDITION(Utf8_CodePointsMatch("a", NULL));

		KN_TEST_ASSERT_FALSE(Utf8_CodePointsMatch("a", "b"));
		KN_TEST_ASSERT_FALSE(Utf8_CodePointsMatch("\xEF\xBF\xBE", "\xEF\xBF"));
		KN_TEST_ASSERT_FALSE(Utf8_CodePointsMatch("\xEF\xBF\xBE", "\xEF\0\xBF"));
	}

	KN_TEST_UNIT("Grapheme equality") {
		Grapheme a;
		Grapheme_Create(&a, "a", 1);
		KN_TEST_ASSERT_EQ_U8(a.byteLength, 1);
		KN_TEST_ASSERT_EQ_U8(a.codePointLength, 1);
		KN_TEST_ASSERT_TRUE(Grapheme_EqualsCodePoints(&a, "a", 1));
		KN_TEST_ASSERT_FALSE(Grapheme_EqualsCodePoints(&a, "b", 1));
		KN_TEST_ASSERT_FALSE(Grapheme_EqualsCodePoints(&a, "ba", 2));
		KN_TEST_ASSERT_FALSE(Grapheme_EqualsCodePoints(&a, "ab", 2));

		Grapheme b;
		Grapheme_Create(&b, "b", 1);
		KN_TEST_ASSERT_EQ_U8(b.byteLength, 1);
		KN_TEST_ASSERT_EQ_U8(b.codePointLength, 1);
		KN_TEST_ASSERT_TRUE(Grapheme_EqualsCodePoints(&b, "b", 1));

		Grapheme extended;
		Grapheme_Create(&extended, "\xe2\x80\xa2\xe2\x88\x99", 2);
		KN_TEST_ASSERT_EQ_U8(extended.byteLength, 6);
		KN_TEST_ASSERT_EQ_U8(extended.codePointLength, 2);
		KN_TEST_ASSERT_TRUE(Grapheme_EqualsCodePoints(&extended, "\xe2\x80\xa2\xe2\x88\x99", 2));
		KN_TEST_ASSERT_FALSE(Grapheme_EqualsCodePoints(&extended, "\xe2\x80\xa2\xe2\x88\x98", 2));
	}

	KN_TEST_UNIT("Grapheme equality") {
		Grapheme a;
		Grapheme_Create(&a, "a", 1);
		KN_TEST_ASSERT_TRUE(Grapheme_EqualsGrapheme(&a, &a));

		Grapheme abc;
		Grapheme_Create(&abc, "abc", 3);
		KN_TEST_ASSERT_TRUE(Grapheme_EqualsGrapheme(&abc, &abc));

		KN_TEST_ASSERT_FALSE(Grapheme_EqualsGrapheme(&a, &abc));

		KN_TEST_PRECONDITION(Grapheme_EqualsGrapheme(NULL, &a));
		KN_TEST_PRECONDITION(Grapheme_EqualsGrapheme(&a, NULL));
	}

	KN_TEST_UNIT("Building code point sequences") {
		Grapheme a;
		Grapheme_Begin(&a);
		Grapheme_AddCodePoint(&a, "a");
		Grapheme_AddCodePoint(&a, "b");
		Grapheme_AddCodePoint(&a, "c");

		Grapheme abc;
		Grapheme_Create(&abc, "abc", 3);
		KN_TEST_ASSERT_TRUE(Grapheme_EqualsGrapheme(&a, &abc));
	}

KN_TEST_SUITE_END
