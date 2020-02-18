/*
 * Loading of PSF version 2 fonts.  PSF is a simple bitmap-based font format
 * using fixed width, which is typically suitable for text consoles.
 *
 * Fixed width fonts prevent needing to worry about kerning in fonts.  These
 * only support a limited number of characters and one resolution, but are
 * quick to use to get off the ground.
 */
#ifndef KN_FONT_PSF2_H
#define KN_FONT_PSF2_H

#include <knell/image.h>

#define MAX_UTF8_CHAR_LENGTH 4

#define KN_MAX_CODE_POINTS_IN_GLYPH 3

/**
 * Makes utf8char printable by adding another byte which can be set to \0.
 */
typedef uint8_t utf8char[MAX_UTF8_CHAR_LENGTH + 1];

#define KN_MAX_FONT_PSF2_GLYPHS 256

/**
 *
 */
typedef struct
{
	ImageRGBA8 pixels;
} FontPSF2Description;

/**
 * Each glyph represents possibly one or more code points.
 */
typedef struct
{
	/**
	 * The code points represented by this glyph.
	 */
	utf8char codePoint[KN_MAX_CODE_POINTS_IN_GLYPH];
	uint8_t codePointsInGlyph;
} GlyphMapping;

/**
 * Describes the relationship between glyphs and code points.
 */
typedef struct {

	/**
	 * Each index maps to it's appropriate glyph.
	 */
	GlyphMapping mapping[KN_MAX_FONT_PSF2_GLYPHS];
} FontPSF2;

KN_API bool Font_PSF2Allocate(ImageRGBA8* description, FontPSF2* font, const char* path);
KN_API bool Font_PSF2GlyphsToPrint(FontPSF2* font, const char* str, uint32_t* glyphs, uint32_t* length);
KN_API uint8_t Font_BytesInUtf8CodePoint(char leadingByte);
KN_API uint32_t Font_CodePointToGlyphIndex(FontPSF2* font, char* codePoint);

#endif // KN_FONT_PSF2_H
