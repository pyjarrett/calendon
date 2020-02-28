#ifndef KN_FONT_UTF8_GLYPH_H
#define KN_FONT_UTF8_GLYPH_H

#include <knell/kn.h>
#include <knell/utf8.h>

typedef uint32_t GlyphIndex;
#define KN_GLYPH_INDEX_INVALID ((uint32_t)(~0))
#define KN_MAX_GLYPH_MAP_GRAPHEMES 512

/**
 * Treated as an opaque pointer by clients.
 */
typedef struct {
	Grapheme graphemes[KN_MAX_GLYPH_MAP_GRAPHEMES];

	// glyph[i] is the glyph for sequence[i]
	GlyphIndex glyphs[KN_MAX_GLYPH_MAP_GRAPHEMES];
	uint32_t usedSequences;
} Utf8GlyphMap;

KN_UNIT_API void Utf8GlyphMap_Create(Utf8GlyphMap* map);
KN_UNIT_API uint32_t Utf8GlyphMap_GlyphForCodePoint(Utf8GlyphMap* map, const uint8_t* codePoint, uint8_t numCodePoints);
KN_UNIT_API bool Utf8GlyphMap_Map(Utf8GlyphMap* map, const uint8_t* codePoint, uint8_t numCodePoints, GlyphIndex glyphIndex);

#endif //KNELL_FONT_UTF8_GLYPH_H
