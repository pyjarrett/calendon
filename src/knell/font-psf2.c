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

#define PSF2_SEPARATOR  0xFF
#define PSF2_SEQEND   0xFE

KN_API uint8_t Font_BytesInUtf8CodePoint(char leadingByte)
{
	/*
	 * The number of bytes in a utf-8 char can be determined by the upper nibble
	 * of the first byte.
	 *
	 * Number of bytes:
	 * 1: 0xxx xxxx : <7
	 * 2: 110x xxxx : (C|D)
	 * 3: 1110 xxxx : E
	 * 4: 1111 0xxx : F
	 */
	const uint8_t b = leadingByte & 0xF0;
	switch (b) {
		case 0xF0: return 4;
		case 0xE0: return 3;
		case 0xD0: // fallthrough
		case 0xC0:
			return 2;
		default: return 1;
	}
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

static void Font_PSF2ReadUnicodeTable(uint8_t* const unicodeTableStart,
	uint8_t* const unicodeTableEnd)
{
	/*
	 * <unicodeimage> := <uc>*<seq>*<term>
	 * <seq> := <ss><uc><uc>*
	 * <ss> := psf1 ? 0xFFFE : 0xFE
	 * <term> := psf1 ? 0xFFFF : 0xFF
	 * <uc :+ psf1 ? 2 byte little endian unicode : UTF-8 value
	 */
	uint8_t* unicodeTableCursor = unicodeTableStart;
	uint32_t glyphIndex = 0;
	uint32_t charIndex = 0;
	CharMap charMap;
	memset(charMap, 0, MAX_UTF8_CHAR_LENGTH * KN_CHAR_MAP_SIZE);
	bool newline = true;
	while (unicodeTableCursor < unicodeTableEnd) {
		if (newline) {
			printf("%3i ", glyphIndex);
			newline = false;
			printf(" CHARS[%i] ", Font_BytesInUtf8CodePoint(*unicodeTableCursor));
		}
		if (*unicodeTableCursor == PSF2_SEPARATOR) {
			++glyphIndex;
			charIndex = 0;
			newline = true;
			printf("\n");
		}
		else if (*unicodeTableCursor == PSF2_SEQEND) {
			// More characters refer to this glyph.
			charIndex = 0;
			printf("    ");
		}
		else {
			printf("%X ", *unicodeTableCursor);
			charMap[glyphIndex][charIndex] = *unicodeTableCursor;
			++charIndex;
		}
		++unicodeTableCursor;
	}

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
	Font_PSF2ReadUnicodeTable(unicodeTableStart, unicodeTableEnd);
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



}
