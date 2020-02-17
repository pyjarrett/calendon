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

/**
 * Makes utf8char printable by adding another byte which can be set to \0.
 */
typedef uint8_t utf8char[MAX_UTF8_CHAR_LENGTH + 1];

// TODO: Rename this.
#define KN_CHAR_MAP_SIZE 256
typedef utf8char CharMap[KN_CHAR_MAP_SIZE];

/**
 *
 */
typedef struct
{
	ImageRGBA8 pixels;
} FontPSF2Description;

typedef struct
{
	/**
	 * The code points represented by this glyph.
	 */
	utf8char codePoint;
	bool matchesAscii;
} GlyphMapping;

/**
 * Describes the relationship between glyphs and code points.
 */
typedef struct {
	/**
	 * Map of available characters to glyph indices.
	 */
	CharMap charMap;

	/**
	 * Each index maps to it's appropriate glyph.
	 */
	GlyphMapping mapping[KN_CHAR_MAP_SIZE];
} FontPSF2;

KN_API bool Font_PSF2Allocate(ImageRGBA8* description, FontPSF2* font, const char* path);
KN_API bool Font_PSF2GlyphsToPrint(FontPSF2* font, const char* str, uint32_t* glyphs, uint32_t* length);
KN_API uint8_t Font_BytesInUtf8CodePoint(char leadingByte);

#endif // KN_FONT_PSF2_H
