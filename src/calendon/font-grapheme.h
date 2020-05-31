#ifndef CN_FONT_UTF8_GLYPH_H
#define CN_FONT_UTF8_GLYPH_H

#include <calendon/cn.h>

#include <calendon/utf8.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t CnGlyphIndex;

/**
 * Used to indicate a grapheme mapping does not exist or it not initialized.
 */
#define CN_GRAPHEME_INDEX_INVALID ((uint32_t)(~0))

/**
 * Indicates maximum compile-time storage requirements for the number of
 * possible graphemes.  This limit exists more to reduce the total memory usage
 * rather than provide a true limit on the number of graphemes which should be
 * supported.
 */
#define CN_MAX_GLYPH_MAP_GRAPHEMES 512

/**
 * Maps UTF-8 encoded graphemes to glyph indices for display by a font with
 * glyphs corresponding to each index.
 */
typedef struct {
	CnGrapheme graphemes[CN_MAX_GLYPH_MAP_GRAPHEMES];

	// glyph[i] is the glyph for sequence[i]
	CnGlyphIndex glyphs[CN_MAX_GLYPH_MAP_GRAPHEMES];
	uint32_t usedGraphemes;
} CnGraphemeMap;

CN_TEST_API void cnGraphemeMap_Clear(CnGraphemeMap* map);
CN_TEST_API uint32_t cnGraphemeMap_GlyphForCodePoints(CnGraphemeMap* map, const uint8_t* codePoint, uint8_t numCodePoints);
CN_TEST_API uint32_t cnGraphemeMap_GraphemeIndexForCodePoints(CnGraphemeMap* map, const uint8_t* codePoint, uint8_t numCodePoints);
CN_TEST_API bool cnGraphemeMap_Map(CnGraphemeMap* map, const uint8_t* codePoint, uint8_t numCodePoints, CnGlyphIndex glyphIndex);

#ifdef __cplusplus
}
#endif

#endif //CALENDON_FONT_UTF8_GLYPH_H
