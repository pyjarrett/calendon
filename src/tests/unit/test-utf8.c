#include <calendon/test.h>

#include <calendon/utf8.h>

#include <stdlib.h>
#include <stdio.h>

CN_TEST_SUITE_BEGIN("UTF-8")

	CN_TEST_UNIT("Bytes in UTF-8 code point") {
		CN_TEST_ASSERT_EQ_U8(cnUtf8_NumBytesInCodePoint('a'), 1);
		CN_TEST_ASSERT_EQ_U8(cnUtf8_NumBytesInCodePoint('\xC2'), 2);
		CN_TEST_ASSERT_EQ_U8(cnUtf8_NumBytesInCodePoint('\xE2'), 3);

		CN_TEST_PRECONDITION(cnUtf8_NumBytesInCodePoint('\xC0'));
		CN_TEST_PRECONDITION(cnUtf8_NumBytesInCodePoint('\xFE'));
		CN_TEST_PRECONDITION(cnUtf8_NumBytesInCodePoint('\xFF'));

		// Continuation, but not leading bytes.
		CN_TEST_PRECONDITION(cnUtf8_NumBytesInCodePoint('\x84'));
	}

	CN_TEST_UNIT("Identification of leading bytes") {
		CN_TEST_ASSERT_TRUE(cnUtf8_IsLeadingByte('\0'));
		CN_TEST_ASSERT_TRUE(cnUtf8_IsLeadingByte('\xEF'));

		for (uint8_t b = 0x80; b <= 0xBF; ++b) {
			CN_TEST_ASSERT_FALSE(cnUtf8_IsLeadingByte(b));
		}

		CN_TEST_PRECONDITION(cnUtf8_IsLeadingByte('\xC0'));
		CN_TEST_PRECONDITION(cnUtf8_IsLeadingByte('\xF5'));
	}

	CN_TEST_UNIT("Code points match happy case") {
		CN_TEST_ASSERT_TRUE(cnUtf8_CodePointsMatch("b", "b"));
		CN_TEST_ASSERT_TRUE(cnUtf8_CodePointsMatch("\xEF\xBF\xBE", "\xEF\xBF\xBE"));
	}

	CN_TEST_UNIT("Code points don't match") {
		CN_TEST_PRECONDITION(cnUtf8_CodePointsMatch(NULL, "b"));
		CN_TEST_PRECONDITION(cnUtf8_CodePointsMatch("a", NULL));

		CN_TEST_ASSERT_FALSE(cnUtf8_CodePointsMatch("a", "b"));
		CN_TEST_ASSERT_FALSE(cnUtf8_CodePointsMatch("\xEF\xBF\xBE", "\xEF\xBF"));

		// Invalid continuation byte
		CN_TEST_ASSERT_FALSE(cnUtf8_CodePointsMatch("\xEF\xBF\xBE", "\xEF\0\xBF"));
	}

	CN_TEST_UNIT("UTF-8 string length") {
		CN_TEST_PRECONDITION(cnUtf8_StringLength(NULL));

		CN_TEST_ASSERT_EQ_SIZE_T(0, cnUtf8_StringLength(""));
		CN_TEST_ASSERT_EQ_SIZE_T(4, cnUtf8_StringLength("test"));
		CN_TEST_ASSERT_EQ_SIZE_T(21, cnUtf8_StringLength("«café, caffè» ™ © Â ←"));
	}

	CN_TEST_UNIT("UTF-8 invalid string length") {
		// String has a continuation byte, but not all expected bytes.
		CN_TEST_PRECONDITION(cnUtf8_StringLength("\xEF"));
	}

	CN_TEST_UNIT("UTF-8 string validity") {
		CN_TEST_ASSERT_TRUE(cnUtf8_IsStringValid(""));

		CN_TEST_ASSERT_TRUE(cnUtf8_IsStringValid("This is a test"));
		CN_TEST_ASSERT_TRUE(cnUtf8_IsStringValid("«café, caffè» ™ © Â ←"));

		CN_TEST_ASSERT_FALSE(cnUtf8_IsStringValid("\xEF\xEF"));
		CN_TEST_ASSERT_FALSE(cnUtf8_IsStringValid("\xBF\xBE"));
	}

	CN_TEST_UNIT("UTF-8 string equality") {
		CN_TEST_ASSERT_TRUE(cnUtf8_StringEqual("«café, caffè» ™ © Â ←", "«café, caffè» ™ © Â ←"));
	}

	CN_TEST_UNIT("UTF-8 string inequality") {
		CN_TEST_ASSERT_FALSE(cnUtf8_StringEqual("«café, caffè» ™ © Â ←", "«cafe, caffe» ™ © Â ←"));

		// Invalid continuation byte.
		CN_TEST_PRECONDITION(cnUtf8_StringEqual("\xEF\xBF\xBE", "\xEF\xBF"));
		CN_TEST_PRECONDITION(cnUtf8_StringEqual("\xEF\xBF", "\xEF\xBF\xBE"));
		CN_TEST_PRECONDITION(cnUtf8_StringEqual("\xEF\0\xBF\xBE", "\xEF\0\xBF\xBE"));
		CN_TEST_PRECONDITION(cnUtf8_StringEqual("\xEF\0\xBF\xBE", "\xEF\xBF\xBE"));
		CN_TEST_PRECONDITION(cnUtf8_StringEqual("\xEF\xBF\xBE", "\xEF\0\xBF\xBE"));
	}

	CN_TEST_UNIT("UTF-8 string next") {
		CN_TEST_PRECONDITION(cnUtf8_StringNext(NULL));
	}

	CN_TEST_UNIT("CnGrapheme equality") {
		CnGrapheme a;
		cnGrapheme_Set(&a, "a", 1);
		CN_TEST_ASSERT_EQ_U8(a.byteLength, 1);
		CN_TEST_ASSERT_EQ_U8(a.codePointLength, 1);
		CN_TEST_ASSERT_TRUE(cnGrapheme_EqualsCodePoints(&a, "a", 1));
		CN_TEST_ASSERT_FALSE(cnGrapheme_EqualsCodePoints(&a, "b", 1));
		CN_TEST_ASSERT_FALSE(cnGrapheme_EqualsCodePoints(&a, "ba", 2));
		CN_TEST_ASSERT_FALSE(cnGrapheme_EqualsCodePoints(&a, "ab", 2));

		CnGrapheme b;
		cnGrapheme_Set(&b, "b", 1);
		CN_TEST_ASSERT_EQ_U8(b.byteLength, 1);
		CN_TEST_ASSERT_EQ_U8(b.codePointLength, 1);
		CN_TEST_ASSERT_TRUE(cnGrapheme_EqualsCodePoints(&b, "b", 1));

		CnGrapheme extended;
		cnGrapheme_Set(&extended, "\xe2\x80\xa2\xe2\x88\x99", 2);
		CN_TEST_ASSERT_EQ_U8(extended.byteLength, 6);
		CN_TEST_ASSERT_EQ_U8(extended.codePointLength, 2);
		CN_TEST_ASSERT_TRUE(cnGrapheme_EqualsCodePoints(&extended, "\xe2\x80\xa2\xe2\x88\x99", 2));
		CN_TEST_ASSERT_FALSE(cnGrapheme_EqualsCodePoints(&extended, "\xe2\x80\xa2\xe2\x88\x98", 2));
	}

	CN_TEST_UNIT("CnGrapheme equality") {
		CnGrapheme a;
		cnGrapheme_Set(&a, "a", 1);
		CN_TEST_ASSERT_TRUE(cnGrapheme_EqualsGrapheme(&a, &a));

		CnGrapheme abc;
		cnGrapheme_Set(&abc, "abc", 3);
		CN_TEST_ASSERT_TRUE(cnGrapheme_EqualsGrapheme(&abc, &abc));

		CN_TEST_ASSERT_FALSE(cnGrapheme_EqualsGrapheme(&a, &abc));

		CN_TEST_PRECONDITION(cnGrapheme_EqualsGrapheme(NULL, &a));
		CN_TEST_PRECONDITION(cnGrapheme_EqualsGrapheme(&a, NULL));
	}

	CN_TEST_UNIT("Building code point sequences") {
		CnGrapheme a;
		cnGrapheme_Begin(&a);
		cnGrapheme_AddCodePoint(&a, "a");
		cnGrapheme_AddCodePoint(&a, "b");
		cnGrapheme_AddCodePoint(&a, "c");

		CnGrapheme abc;
		cnGrapheme_Set(&abc, "abc", 3);
		CN_TEST_ASSERT_TRUE(cnGrapheme_EqualsGrapheme(&a, &abc));
	}

CN_TEST_SUITE_END
