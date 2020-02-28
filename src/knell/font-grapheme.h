#ifndef KN_FONT_UTF8_GLYPH_H
#define KN_FONT_UTF8_GLYPH_H

#include <knell/kn.h>
#include <knell/utf8.h>

typedef uint32_t GlyphIndex;

/**
 * Used to indicate a grapheme mapping does not exist or it not initialized.
 */
#define KN_GLYPH_INDEX_INVALID ((uint32_t)(~0))

/**
 * Indicates maximum compile-time storage requirements for the number of
 * possible graphemes.  This limit exists more to reduce the total memory usage
 * rather than provide a true limit on the number of graphemes which should be
 * supported.
 */
#define KN_MAX_GLYPH_MAP_GRAPHEMES 512

/**
 * Maps UTF-8 encoded graphemes to glyph indices for display by a font with
 * glyphs corresponding to each index.
 */
typedef struct {
	Grapheme graphemes[KN_MAX_GLYPH_MAP_GRAPHEMES];

	// glyph[i] is the glyph for sequence[i]
	GlyphIndex glyphs[KN_MAX_GLYPH_MAP_GRAPHEMES];
	uint32_t usedGraphemes;
} GraphemeMap;

KN_TEST_API void GraphemeMap_Create(GraphemeMap* map);
KN_TEST_API uint32_t GraphemeMap_GlyphForCodePoints(GraphemeMap* map, const uint8_t* codePoints, uint8_t numCodePoints);
KN_TEST_API bool GraphemeMap_Map(GraphemeMap* map, const uint8_t* codePoints, uint8_t numCodePoints, GlyphIndex glyphIndex);

#endif //KNELL_FONT_UTF8_GLYPH_H
