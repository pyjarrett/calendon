#include <knell/font-psf2.h>

#include <knell/kn.h>
#include <knell/assets-fileio.h>
#include <knell/log.h>

#include <string.h>

//https://www.win.tue.nl/~aeb/linux/kbd/font-formats-1.html
static uint8_t psf2Magic[4] = { 0x72, 0xb5, 0x4a, 0x86 };

/* bits used in flags */
#define PSF2_HAS_UNICODE_TABLE 0x01

/* max version recognized so far */
#define PSF2_MAXVERSION 0

/**
 * Appears between code point sequences to indicate an identical sequence
 * mapping to the same glyph.
 */
#define PSF2_SEPARATOR   0xFE
#define PSF2_TERM  0xFF

/**
 * To draw a glyph, we need to know which glyph we're trying to draw.
 */
KN_API uint32_t Font_CodePointToGlyphIndex(FontPSF2* font, const char* codePoint)
{
	KN_ASSERT(font != NULL, "Cannot find a glyph index for a null font");
	KN_ASSERT(codePoint != NULL, "Cannot get the glyph index for a code point.");

	// One of these glyphs should
	for (uint32_t i=0; i < KN_MAX_FONT_PSF2_GLYPHS; ++i) {
		font->mapping[i].codePointsInGlyph;
		font->mapping[i].codePoint[0];
	}
	font->mapping;
	return 0;
}


/**
 * Binary layout of the PSF2 file header.
 */
typedef struct {
	unsigned char magic[4];
	uint32_t version;
	uint32_t bitmapOffset;
	uint32_t flags;
	uint32_t numGlyphs;
	uint32_t bytesPerGlyph;
	uint32_t glyphHeight, glyphWidth; /* max dimensions of glyphs */
	/* charsize = height * ((width + 7) / 8) */
} PSF2Header;

void Font_PSF2PrintHeader(const PSF2Header *header)
{
	KN_TRACE(LogSysMain, "Version is %" PRIu32, header->version);
	KN_TRACE(LogSysMain, "Header is %" PRIu32 " bytes", header->bitmapOffset);
	KN_TRACE(LogSysMain, "%" PRIu32 " glyphs", header->numGlyphs);
	KN_TRACE(LogSysMain, "%" PRIu32 " bytes per glyph", header->numGlyphs);
	KN_TRACE(LogSysMain, "%" PRIu32 "x%" PRIu32, header->glyphWidth, header->glyphHeight);
}

static size_t ReadGrapheme(Utf8GlyphMap* map, GlyphIndex glyphIndex,
	const uint8_t* const unicodeTableStart, const uint8_t* const unicodeTableEnd)
{
	KN_ASSERT(map != NULL, "Cannot read a grapheme into a null glyph map.");
	KN_ASSERT(unicodeTableStart != NULL, "Cannot read a unicode from a null unicode table.");
	KN_ASSERT(unicodeTableEnd != NULL, "Unicode table to read from has no end.");
	KN_ASSERT(unicodeTableStart < unicodeTableEnd, "Unicode table begins after it ends.");

	Grapheme g;
	Grapheme_Begin(&g);

	size_t totalBytesRead = 0;

	const uint8_t* cursor = unicodeTableStart;
	while (cursor < unicodeTableEnd && *cursor != PSF2_SEPARATOR && *cursor != PSF2_TERM) {
		// TODO: Check for invalid code points.
		Grapheme_AddCodePoint(&g, (char*)cursor);
		const size_t bytesInCodePoint = Utf8_NumBytesInCodePoint(*cursor);
		cursor += bytesInCodePoint;
		totalBytesRead += bytesInCodePoint;
	}
	Utf8GlyphMap_Map(map, &g.codePoints[0], g.codePointLength, glyphIndex);
	Grapheme_Print(&g, stdout);
	printf("\n");
	//KN_TRACE(LogSysMain, "Read");
	return totalBytesRead;
}

static void Font_PSF2ReadUnicodeTableAsGlyphMap(Utf8GlyphMap* map,
	uint8_t* const unicodeTableStart, uint8_t* const unicodeTableEnd)
{
	KN_ASSERT(unicodeTableStart < unicodeTableEnd, "Unicode table ends before it starts");

	KN_TRACE(LogSysMain, "Size of Glyph Map: %" PRIu64 "\n", sizeof(Utf8GlyphMap));
	KN_TRACE(LogSysMain, "Size of FontPSF2: %" PRIu64 "\n", sizeof(FontPSF2));
	KN_TRACE(LogSysMain, "Reading unicode table of %zu bytes", (unicodeTableEnd - unicodeTableStart));
	Utf8GlyphMap_Create(map);
	uint8_t* cursor = unicodeTableStart;
	GlyphIndex glyphIndex = 0;

	/*
	 * <unicodeimage> := <uc>*<seq>*<term>
	 * <seq> := <ss><uc><uc>*
	 * <ss> := psf1 ? 0xFFFE : 0xFE
	 * <term> := psf1 ? 0xFFFF : 0xFF
	 * <uc :+ psf1 ? 2 byte little endian unicode : UTF-8 value
	 */
	while (cursor < unicodeTableEnd) {
		// Read the first glyph sequence.
		const size_t initialBytesRead = ReadGrapheme(map, glyphIndex, cursor, unicodeTableEnd);
		cursor += initialBytesRead;

		// Look for a separator or terminator.
		while (*cursor == PSF2_SEPARATOR) {
			const size_t moreBytesRead = ReadGrapheme(map, glyphIndex, cursor, unicodeTableEnd);
			cursor += moreBytesRead;
		}

		KN_ASSERT(*cursor == PSF2_TERM, "Unicode table grapheme doesn't end in terminator.");
		++glyphIndex;
		++cursor;
	}

	KN_ASSERT(cursor == unicodeTableEnd, "Cursor didn't end at correct point: %" PRIiPTR, (intptr_t)(unicodeTableEnd - cursor));
	// TODO: Check that all glyph indices have been read.
}

/**
 * Creates the suitable elements needed to display a font.  This includes maps
 * for determining which glyphs to use, and the appropriate texture with which
 * to draw the font.
 *
 * Font loading resolves many questions related to the font:
 * - Which characters are supported by the font?
 * - Which glyphs should be drawn by a given string?
 * - How many glyphs are in a string?
 * - What is the width and height of a given string?
 */
KN_API bool Font_PSF2Allocate(ImageRGBA8* image, FontPSF2* font, const char* path)
{
	DynamicBuffer fileBuffer;
	if (!Assets_ReadFile(path, KN_FILE_TYPE_BINARY, &fileBuffer)) {
		KN_FATAL_ERROR("Unable to load font from %s", path);
	}

	const PSF2Header* const header = (PSF2Header*)fileBuffer.contents;

	// Check magic bytes to ensure that it's the correct file format.
	if (memcpy(psf2Magic, header->magic, 4) == 0) {
		KN_ERROR(LogSysMain, "Bad Magic Found: %x %x %x %x Expected: %x %x %x %x",
			header->magic[0], header->magic[1], header->magic[2], header->magic[3],
			psf2Magic[0], psf2Magic[1], psf2Magic[2], psf2Magic[3]);
	}
	Font_PSF2PrintHeader(header);

	// Requirement imposed by Knell to ensure byte boundaries on generated texture.
	KN_ASSERT((header->glyphWidth & 7) == 0, "Character width is not evenly divisible by 8");
	KN_ASSERT((header->glyphHeight & 7) == 0, "Character height is not evenly divisible by 8");

	font->glyphSize.width = header->glyphWidth;
	font->glyphSize.height = header->glyphHeight;

	// TODO: Split this out into a function.
	// The bitmap is recorded after the header.
	uint8_t* const bitmapStart = (uint8_t*)header + header->bitmapOffset;
	const uint32_t bitmapSize = header->bytesPerGlyph * header->numGlyphs;
	{
		uint8_t* bitmapCursor = (uint8_t*)header + header->bitmapOffset;

		const uint32_t bytesPerPixel = 4;
		Mem_Allocate(&image->pixels, header->numGlyphs * bytesPerPixel * header->glyphWidth * header->glyphHeight);
		memset(image->pixels.contents, 0, image->pixels.size);

		// TODO: Document RGBA8 assumption.
		uint32_t* imageCursor = (uint32_t*)image->pixels.contents;
		uint32_t* imageEnd = (uint32_t*)((uint8_t*)imageCursor + image->pixels.size);

		// The bitmap for a glyph is stored as height consecutive pixel rows,
		// where each pixel row consists of width bits followed by some filler
		// bits in order to fill an integral number of (8-bit) bytes.
		//
		// Parse all characters.
		for (uint32_t i = 0; i < header->numGlyphs; ++i) {
			// Parse all rows of the next character.
			for (uint32_t row = 0; row < header->glyphHeight; ++row) {
				// Parse the next row.
				for (uint32_t col = 0; col < (header->glyphWidth / 8); ++col) {
					const uint8_t nextByte = *bitmapCursor;
					for (int32_t bit = 7; bit >= 0; --bit) {
						if ((1 << bit) & nextByte) {
							//printf("X");
							// Set all the bits to 1.
							*imageCursor = (uint32_t)~0;
						}
						else {
							//printf(" ");
						}
						++imageCursor;
					}
					++bitmapCursor;
					//printf("_");
				}
				//printf("\n");
			}
		}
		KN_ASSERT(imageCursor == imageEnd, "Didn't count every pixel");
	}

	if (header->flags & PSF2_HAS_UNICODE_TABLE) {
		KN_TRACE(LogSysMain, "Has a unicode table");
	}
	else {
		// If there is no unicode table, there is no way to determine which
		// glyph maps to which character.
		KN_TRACE(LogSysMain, "No unicode table");
		return false;
	}
	uint8_t* const unicodeTableStart = bitmapStart + bitmapSize;
	uint8_t* const unicodeTableEnd = (uint8_t*)fileBuffer.contents + fileBuffer.size;
	Font_PSF2ReadUnicodeTableAsGlyphMap(&font->map, unicodeTableStart, unicodeTableEnd);
	Mem_Free(&image->pixels);
	Mem_Free(&fileBuffer);
	return true;
}

/**
 * Given a font mapping characters to glyph indices, find the glyphs which
 * should be printed.
 *
 * @param length location to write out the number of glyphs being printed.
 */
KN_API bool Font_PSF2GlyphsToPrint(FontPSF2* font, const char* str,
	uint32_t* glyphs, uint32_t* length)
{
	KN_ASSERT(font != NULL, "Cannot determine glyphs to print for a null font.");
	KN_ASSERT(str != NULL, "Cannot print a null string.");
	KN_ASSERT(glyphs != NULL, "Cannot writes glyphs to a null location.");
	KN_ASSERT(length != NULL, "No location to which to write glyphs.");

	uint32_t currentGlyph = 0;
	char* currentCodePoint = str;
	char* afterLastCodePoint = str + strlen(str);
	while (currentCodePoint < afterLastCodePoint) {
		// Is the code point in the glyph map?
		for (uint32_t i=0; i < font->mapping[i].codePointsInGlyph; ++i) {
			if (Utf8_CodePointsMatch(str, font->mapping[i].codePoint[i]));
		}
		++currentGlyph;
	}
	*length = currentGlyph;
	return false;
}
