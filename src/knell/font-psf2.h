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

#include <knell/font-grapheme.h>
#include <knell/image.h>
#include <knell/math2.h>

/**
 * Describes the relationship between glyphs and code points.
 */
typedef struct {
	GraphemeMap map;
	dimension2u32 glyphSize;
} FontPSF2;

KN_API bool Font_PSF2Allocate(FontPSF2* font, ImageRGBA8* description, const char* path);

#endif // KN_FONT_PSF2_H
