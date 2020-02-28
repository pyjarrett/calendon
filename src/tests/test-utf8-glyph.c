#include <knell/test.h>

#include <knell/font-glyph.h>

KN_TEST_SUITE_BEGIN("font-utf8-glyph")
	printf("Size of Utf8GlyphMap: %zu bytes\n", sizeof(Utf8GlyphMap));

	KN_TEST_UNIT("Unicode table creation") {
		Utf8GlyphMap map;
		Utf8GlyphMap_Create(&map);
	}

	KN_TEST_UNIT("Glyph mapping") {
		Utf8GlyphMap map;
		Utf8GlyphMap_Create(&map);
		Utf8GlyphMap_Map(&map, "a", 1, 5);
		KN_TEST_ASSERT_EQ_U32(5, Utf8GlyphMap_GlyphForCodePoint(&map, "a", 1));
	}

KN_TEST_SUITE_END
