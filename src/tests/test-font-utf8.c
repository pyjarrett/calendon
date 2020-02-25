#include <knell/test.h>

#include <knell/font-utf8.h>

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

	KN_TEST_UNIT("Code point sequence equality") {
		CodePointSequence a;
		CodePointSequence_Create(&a, "a", 1);
		KN_TEST_ASSERT_EQ_U8(a.byteLength, 1);
		KN_TEST_ASSERT_EQ_U8(a.sequenceLength, 1);
		KN_TEST_ASSERT_TRUE(CodePointSequence_Is(&a, "a", 1));
		KN_TEST_ASSERT_FALSE(CodePointSequence_Is(&a, "b", 1));
		KN_TEST_ASSERT_FALSE(CodePointSequence_Is(&a, "ba", 2));
		KN_TEST_ASSERT_FALSE(CodePointSequence_Is(&a, "ab", 2));

		CodePointSequence b;
		CodePointSequence_Create(&b, "b", 1);
		KN_TEST_ASSERT_EQ_U8(b.byteLength, 1);
		KN_TEST_ASSERT_EQ_U8(b.sequenceLength, 1);
		KN_TEST_ASSERT_TRUE(CodePointSequence_Is(&b, "b", 1));

		CodePointSequence extended;
		CodePointSequence_Create(&extended, "\xe2\x80\xa2\xe2\x88\x99", 2);
		KN_TEST_ASSERT_EQ_U8(extended.byteLength, 6);
		KN_TEST_ASSERT_EQ_U8(extended.sequenceLength, 2);
		KN_TEST_ASSERT_TRUE(CodePointSequence_Is(&extended, "\xe2\x80\xa2\xe2\x88\x99", 2));
		KN_TEST_ASSERT_FALSE(CodePointSequence_Is(&extended, "\xe2\x80\xa2\xe2\x88\x98", 2));
	}

	KN_TEST_UNIT("CodePointSequence equality") {
		CodePointSequence a;
		CodePointSequence_Create(&a, "a", 1);
		KN_TEST_ASSERT_TRUE(CodePointSequence_Equal(&a, &a));

		CodePointSequence abc;
		CodePointSequence_Create(&abc, "abc", 3);
		KN_TEST_ASSERT_TRUE(CodePointSequence_Equal(&abc, &abc));

		KN_TEST_ASSERT_FALSE(CodePointSequence_Equal(&a, &abc));

		KN_TEST_PRECONDITION(CodePointSequence_Equal(NULL, &a));
		KN_TEST_PRECONDITION(CodePointSequence_Equal(&a, NULL));
	}

	KN_TEST_UNIT("Building code point sequences") {
		CodePointSequence a;
		CodePointSequence_Begin(&a, "a");
		CodePointSequence_AddCodePoint(&a, "b");
		CodePointSequence_AddCodePoint(&a, "c");

		CodePointSequence abc;
		CodePointSequence_Create(&abc, "abc", 3);
		KN_TEST_ASSERT_TRUE(CodePointSequence_Equal(&a, &abc));
	}

KN_TEST_SUITE_END
