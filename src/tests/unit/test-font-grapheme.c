#include <knell/test.h>

#include <knell/font-grapheme.h>

KN_TEST_SUITE_BEGIN("font-utf8-graheme")
	printf("Size of GraphemeMap: %zu bytes\n", sizeof(GraphemeMap));

	KN_TEST_UNIT("Unicode table creation") {
		GraphemeMap map;
		GraphemeMap_Create(&map);
	}

	KN_TEST_UNIT("Grapheme mapping of ASCII to ASCII valued glyphs") {
		GraphemeMap map;
		GraphemeMap_Create(&map);

		for (uint8_t i = 0; i < 128; ++i ) {
			const char asciiChar = (char)i;
			GraphemeMap_Map(&map, &asciiChar, 1, i);
		}

		for (uint8_t i = 0; i < 128; ++i ) {
			const char asciiChar = (char)i;
			KN_TEST_ASSERT_EQ_U32((uint32_t)i, GraphemeMap_GlyphForCodePoints(&map, &asciiChar, 1));
		}
	}

KN_TEST_SUITE_END
