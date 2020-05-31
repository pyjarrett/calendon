#include <calendon/test.h>

#include <calendon/font-grapheme.h>

#if defined(__clang__)
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wpointer-sign"
#endif

CN_TEST_SUITE_BEGIN("font-utf8-grapheme")
	printf("Size of CnGraphemeMap: %zu bytes\n", sizeof(CnGraphemeMap));

	CN_TEST_UNIT("Unicode table creation") {
		static CnGraphemeMap map;
		cnGraphemeMap_Clear(&map);
	}

	CN_TEST_UNIT("CnGrapheme mapping of ASCII to ASCII valued glyphs") {
		static CnGraphemeMap map;
		cnGraphemeMap_Clear(&map);

		for (uint8_t i = 0; i < 128; ++i) {
			const char asciiChar = (char)i;
			cnGraphemeMap_Map(&map, &asciiChar, 1, i);
		}

		for (uint8_t i = 0; i < 128; ++i) {
			const char asciiChar = (char)i;
			CN_TEST_ASSERT_EQ_U32((uint32_t)i, cnGraphemeMap_GlyphForCodePoints(&map, &asciiChar, 1));
		}
	}

CN_TEST_SUITE_END

#if defined(__clang__)
	#pragma clang diagnostic pop
#endif

