#include <knell/test.h>

#include <knell/font-glyph.h>

KN_TEST_SUITE_BEGIN("font-utf8-glyph")
	printf("Size of Utf8GraphemeMap: %zu bytes\n", sizeof(Utf8GraphemeMap));

	KN_TEST_UNIT("Unicode table creation") {
		Utf8GraphemeMap map;
		Utf8GraphemeMap_Create(&map);
	}

	KN_TEST_UNIT("Glyph mapping") {
		Utf8GraphemeMap map;
		Utf8GraphemeMap_Create(&map);
		Utf8GraphemeMap_Map(&map, "a", 1, 5);
		KN_TEST_ASSERT_EQ_U32(5, Utf8GraphemeMap_GlyphForCodePoints(&map, "a", 1));
	}

KN_TEST_SUITE_END
