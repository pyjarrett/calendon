#include "font-grapheme.h"

#include <knell/log.h>

#include <string.h>

KN_TEST_API void GraphemeMap_Clear(GraphemeMap* map)
{
	KN_ASSERT(map != NULL, "Cannot create a null grapheme map.");
	memset(map, 0, sizeof(GraphemeMap));
	map->usedGraphemes = 0;
}

KN_TEST_API uint32_t GraphemeMap_GlyphForCodePoints(GraphemeMap* map,
	const KNchar* codePoint, uint8_t numCodePoints)
{
	KN_ASSERT(map != NULL, "Cannot map to glyphs with a null grapheme map.");
	KN_ASSERT(codePoint != NULL, "Cannot map a null list of graphemes to glyphs.");
	KN_ASSERT(numCodePoints < KN_MAX_CODE_POINTS_IN_GRAPHEME,
			  "Trying to map a sequence of too many code points.");

	for (uint32_t i = 0; i < map->usedGraphemes; ++i) {
		if (Grapheme_EqualsCodePoints(&map->graphemes[i], codePoint, numCodePoints)) {
			return map->glyphs[i];
		}
	}
	return KN_GRAPHEME_INDEX_INVALID;
}

KN_TEST_API uint32_t GraphemeMap_GraphemeIndexForCodePoints(GraphemeMap* map,
	const KNchar* codePoint, uint8_t numCodePoints)
{
	KN_ASSERT(map != NULL, "Cannot map to glyphs with a null grapheme map.");
	KN_ASSERT(codePoint != NULL, "Cannot map a null list of graphemes to glyphs.");
	KN_ASSERT(numCodePoints < KN_MAX_CODE_POINTS_IN_GRAPHEME,
		"Trying to map a sequence of too many code points.");

	for (uint32_t i = 0; i < map->usedGraphemes; ++i) {
		if (Grapheme_EqualsCodePoints(&map->graphemes[i], codePoint, numCodePoints)) {
			return i;
		}
	}
	return KN_GRAPHEME_INDEX_INVALID;
}

/**
 * Creates a grapheme mapping for a sequence of code points.
 */
KN_TEST_API bool GraphemeMap_Map(GraphemeMap* map, const KNchar* codePoint,
	uint8_t numCodePoints, GlyphIndex glyphIndex)
{
	KN_ASSERT(map != NULL, "Cannot write a glyph to a null map.");
	KN_ASSERT(numCodePoints < KN_MAX_CODE_POINTS_IN_GRAPHEME,
			  "Trying to create a mapping for too many code points: %" PRIu8,
		numCodePoints);

	// TODO: Should check for subsequences or larger sequences already existing in the map.
	// Already exists.
	const GlyphIndex existingMapping = GraphemeMap_GlyphForCodePoints(map, codePoint, numCodePoints);
	if (existingMapping == glyphIndex) {
		char graphemeString[KN_MAX_BYTES_IN_GRAPHEME + 1];
		memset(graphemeString, 0, KN_MAX_BYTES_IN_GRAPHEME + 1);
		memcpy(graphemeString, codePoint, numCodePoints);
		KN_WARN(LogSysMain, "Creating a duplicate grapheme mapping for %s", graphemeString);
		return true;
	}
	else if (existingMapping != KN_GRAPHEME_INDEX_INVALID) {
		char graphemeString[KN_MAX_BYTES_IN_GRAPHEME + 1];
		memset(graphemeString, 0, KN_MAX_BYTES_IN_GRAPHEME + 1);
		memcpy(graphemeString, codePoint, numCodePoints);
		KN_WARN(LogSysMain, "Overwriting a previous grapheme mapping for %s", graphemeString);
		return true;
	}

	// All graphemes already used.
	if (map->usedGraphemes + 1 == KN_MAX_GLYPH_MAP_GRAPHEMES) {
		return false;
	}

	Grapheme_Set(&map->graphemes[map->usedGraphemes], codePoint, numCodePoints);
	map->glyphs[map->usedGraphemes] = glyphIndex;
	++map->usedGraphemes;

	return true;
}
