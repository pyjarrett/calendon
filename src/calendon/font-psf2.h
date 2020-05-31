/*
 * Loading of PSF version 2 fonts.  PSF is a simple bitmap-based font format
 * using fixed width, which is typically suitable for text consoles.
 *
 * Fixed width fonts prevent needing to worry about kerning in fonts.  These
 * only support a limited number of characters and one resolution, but are
 * quick to use to get off the ground.
 */
#ifndef CN_FONT_PSF2_H
#define CN_FONT_PSF2_H

#include <calendon/atlas.h>
#include <calendon/font-grapheme.h>
#include <calendon/math2.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Describes the relationship between glyphs and code points.
 */
typedef struct {
	CnGraphemeMap map;
	CnDimension2u32 glyphSize;
	CnTextureAtlas atlas;
} CnFontPSF2;

CN_API bool cnFont_PSF2Allocate(CnFontPSF2* font, const char* path);
CN_API void cnFont_PSF2Free(CnFontPSF2* font);

#ifdef __cplusplus
}
#endif

#endif // CN_FONT_PSF2_H
