#include <knell/font-utf8-glyph.h>
#include <knell/log.h>

KN_UNIT_API void Utf8GlyphMap_Create(Utf8GlyphMap* map)
{
	KN_ASSERT(map != NULL, "Cannot create a null glyph map.");
	map->usedSequences = 0;
}

KN_UNIT_API uint32_t Utf8GlyphMap_GlyphForCodePoint(Utf8GlyphMap* map, const char* codePoint, uint8_t numCodePoints)
{
	KN_ASSERT(map != NULL, "Cannot map to glyphs with a null glyph map.");
	KN_ASSERT(codePoint != NULL, "Cannot map a null list of code points to glyphs.");
	KN_ASSERT(numCodePoints < KN_MAX_CODE_POINTS_IN_SEQUENCE,
			  "Trying to map a sequence of too many code points.");

	for (uint32_t i = 0; i < map->usedSequences; ++i) {
		if (Grapheme_Is(&map->sequences[i], codePoint, numCodePoints)) {
			return map->glyphs[i];
		}
	}
	return KN_GLYPH_INDEX_INVALID;
}

/**
 * Creates a glyph mapping for a sequence of code points.
 */
KN_UNIT_API bool Utf8GlyphMap_Map(Utf8GlyphMap* map, const char* codePoint, uint8_t numCodePoints, GlyphIndex glyphIndex)
{
	KN_ASSERT(map != NULL, "Cannot write a glyph to a null map.");
	KN_ASSERT(numCodePoints < KN_MAX_CODE_POINTS_IN_SEQUENCE,
			  "Trying to create a mapping for too many code points: %" PRIu8,
		numCodePoints);

	// TODO: Should check for subsequences or larger sequences already existing in the map.
	// Already exists.
	const GlyphIndex existingMapping = Utf8GlyphMap_GlyphForCodePoint(map, codePoint, numCodePoints);
	if (existingMapping == glyphIndex) {
		char codePointString[KN_MAX_UTF8_CODE_POINT_BYTE_LENGTH * KN_MAX_CODE_POINTS_IN_GLYPH + 1];
		memset(codePointString, 0, KN_MAX_UTF8_CODE_POINT_BYTE_LENGTH * KN_MAX_CODE_POINTS_IN_GLYPH + 1);
		memcpy(codePointString, codePoint, numCodePoints);
		KN_WARN(LogSysMain, "Creating a duplicate glyph mapping for %s", codePointString);
		return true;
	}
	else if (existingMapping != KN_GLYPH_INDEX_INVALID) {
		char codePointString[KN_MAX_UTF8_CODE_POINT_BYTE_LENGTH * KN_MAX_CODE_POINTS_IN_GLYPH + 1];
		memset(codePointString, 0, KN_MAX_UTF8_CODE_POINT_BYTE_LENGTH * KN_MAX_CODE_POINTS_IN_GLYPH + 1);
		memcpy(codePointString, codePoint, numCodePoints);
		KN_WARN(LogSysMain, "Overwriting a previous glyph mapping for %s", codePointString);
		return true;
	}

	// All sequences already used.
	if (map->usedSequences + 1 == KN_MAX_GLYPH_MAP_CODE_POINT_SEQUENCES) {
		return false;
	}

	Grapheme_Create(&map->sequences[map->usedSequences], codePoint, numCodePoints);
	map->glyphs[map->usedSequences] = glyphIndex;
	++map->usedSequences;

	return true;
}
