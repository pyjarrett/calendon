#include "font-grapheme.h"

#include <calendon/log.h>

#include <string.h>

CN_TEST_API void cnGraphemeMap_Clear(CnGraphemeMap* map)
{
	CN_ASSERT(map != NULL, "Cannot create a null grapheme map.");
	memset(map, 0, sizeof(CnGraphemeMap));
	map->usedGraphemes = 0;
}

CN_TEST_API uint32_t cnGraphemeMap_GlyphForCodePoints(CnGraphemeMap* map,
	const uint8_t* codePoint, uint8_t numCodePoints)
{
	CN_ASSERT(map != NULL, "Cannot map to glyphs with a null grapheme map.");
	CN_ASSERT(codePoint != NULL, "Cannot map a null list of graphemes to glyphs.");
	CN_ASSERT(numCodePoints < CN_MAX_CODE_POINTS_IN_GRAPHEME,
		"Trying to map a sequence of too many code points.");

	for (uint32_t i = 0; i < map->usedGraphemes; ++i) {
		if (cnGrapheme_EqualsCodePoints(&map->graphemes[i], codePoint, numCodePoints)) {
			return map->glyphs[i];
		}
	}
	return CN_GRAPHEME_INDEX_INVALID;
}

CN_TEST_API uint32_t cnGraphemeMap_GraphemeIndexForCodePoints(CnGraphemeMap* map,
	const uint8_t* codePoint, uint8_t numCodePoints)
{
	CN_ASSERT(map != NULL, "Cannot map to glyphs with a null grapheme map.");
	CN_ASSERT(codePoint != NULL, "Cannot map a null list of graphemes to glyphs.");
	CN_ASSERT(numCodePoints < CN_MAX_CODE_POINTS_IN_GRAPHEME,
		"Trying to map a sequence of too many code points.");

	for (uint32_t i = 0; i < map->usedGraphemes; ++i) {
		if (cnGrapheme_EqualsCodePoints(&map->graphemes[i], codePoint, numCodePoints)) {
			return i;
		}
	}
	return CN_GRAPHEME_INDEX_INVALID;
}

/**
 * Creates a grapheme mapping for a sequence of code points.
 */
CN_TEST_API bool cnGraphemeMap_Map(CnGraphemeMap* map, const uint8_t* codePoint,
	int8_t numCodePoints, CnGlyphIndex glyphIndex)
{
	CN_ASSERT(map != NULL, "Cannot write a glyph to a null map.");
	CN_ASSERT(numCodePoints < CN_MAX_CODE_POINTS_IN_GRAPHEME,
		"Trying to create a mapping for too many code points: %" PRIu8, numCodePoints);

	// TODO: Should check for subsequences or larger sequences already existing in the map.
	// Already exists.
	const CnGlyphIndex existingMapping = cnGraphemeMap_GlyphForCodePoints(map, codePoint, numCodePoints);
	if (existingMapping == glyphIndex) {
		char graphemeString[CN_MAX_BYTES_IN_GRAPHEME + 1];
		memset(graphemeString, 0, CN_MAX_BYTES_IN_GRAPHEME + 1);
		memcpy(graphemeString, codePoint, numCodePoints);
		CN_WARN(LogSysMain, "Creating a duplicate grapheme mapping for %s", graphemeString);
		return true;
	}
	else if (existingMapping != CN_GRAPHEME_INDEX_INVALID) {
		char graphemeString[CN_MAX_BYTES_IN_GRAPHEME + 1];
		memset(graphemeString, 0, CN_MAX_BYTES_IN_GRAPHEME + 1);
		memcpy(graphemeString, codePoint, numCodePoints);
		CN_WARN(LogSysMain, "Overwriting a previous grapheme mapping for %s", graphemeString);
		return true;
	}

	// All graphemes already used.
	if (map->usedGraphemes + 1 == CN_MAX_GLYPH_MAP_GRAPHEMES) {
		return false;
	}

	cnGrapheme_Set(&map->graphemes[map->usedGraphemes], codePoint, numCodePoints);
	map->glyphs[map->usedGraphemes] = glyphIndex;
	++map->usedGraphemes;

	return true;
}
