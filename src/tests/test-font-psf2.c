#include <knell/test.h>

#include <knell/assets.h>
#include <knell/log.h>
#include <knell/font-constants.h>
#include <knell/font-psf2.h>
#include <knell/image.h>

typedef uint32_t GlyphIndex;
#define KN_GLYPH_INDEX_INVALID ((uint32_t)(~0))
#define KN_MAX_CODE_POINT_SEQUENCE_LENGTH 2
#define KN_MAX_GLYPH_MAP_CODE_POINT_SEQUENCES 512

/**
 * A sequence of code points is represents a single glyph.
 *
 * An example would be an accent applied to an e.
 */
typedef struct {
	// TODO: Excessive storage used here.
	char codePoints[KN_MAX_UTF8_CODE_POINT_BYTE_LENGTH * KN_MAX_CODE_POINT_SEQUENCE_LENGTH];
	uint8_t sequenceLength;
	uint32_t byteLength;
} CodePointSequence;

void CodePointSequence_Create(CodePointSequence* seq, const char* codePoint, uint8_t numCodePoints)
{
	KN_ASSERT(seq != NULL, "Cannot create a null CodePointSequence.");
	KN_ASSERT(codePoint != NULL, "Cannot create a CodePointSequence from a null code point.");
	KN_ASSERT(numCodePoints <= KN_MAX_CODE_POINT_SEQUENCE_LENGTH,
		"Too many code points provided for a CodePointSequence: %" PRIu8,
		numCodePoints);

	const char* currentCodePoint = codePoint;
	uint8_t currentCodePointIndex = 0;
	uint8_t usedBytes = 0;
	while (currentCodePointIndex < numCodePoints) {
		const uint8_t bytesInCodePoint = Font_BytesInUtf8CodePoint(*currentCodePoint);
		for (uint8_t i = 0; i < bytesInCodePoint; ++i) {
			seq->codePoints[usedBytes++] = currentCodePoint[i];
		}
		// Go the next code point.
		currentCodePoint = currentCodePoint + bytesInCodePoint;
		++currentCodePointIndex;
	}
	seq->byteLength = usedBytes;
	seq->sequenceLength = numCodePoints;
}

bool CodePointSequence_Is(CodePointSequence* seq, const char* codePoint, uint8_t numCodePoints)
{
	const char* currentCodePoint = codePoint;
	uint8_t currentCodePointIndex = 0;
	uint8_t usedBytes = 0;
	while (currentCodePointIndex < numCodePoints) {
		const uint8_t bytesInCodePoint = Font_BytesInUtf8CodePoint(*currentCodePoint);
		for (uint8_t i = 0; i < bytesInCodePoint; ++i) {
			if (seq->codePoints[usedBytes++] != currentCodePoint[i]) {
				return false;
			}
		}
		// Go the next code point.
		currentCodePoint = currentCodePoint + bytesInCodePoint;
		++currentCodePointIndex;
	}
	return true;
}

/**
 * Treated as an opaque pointer by clients.
 */
typedef struct {
	CodePointSequence sequences[KN_MAX_GLYPH_MAP_CODE_POINT_SEQUENCES];

	// glyph[i] is the glyph for sequence[i]
	GlyphIndex glyphs[KN_MAX_GLYPH_MAP_CODE_POINT_SEQUENCES];
	uint32_t usedSequences;
} Utf8GlyphMap;

void Utf8GlyphMap_Create(Utf8GlyphMap* map)
{
	KN_ASSERT(map != NULL, "Cannot create a null glyph map.");
	map->usedSequences = 0;
}

uint32_t Utf8GlyphMap_GlyphForCodePoint(Utf8GlyphMap* map, const char* codePoint, uint8_t numCodePoints)
{
	KN_ASSERT(map != NULL, "Cannot map to glyphs with a null glyph map.");
	KN_ASSERT(codePoint != NULL, "Cannot map a null list of code points to glyphs.");
	KN_ASSERT(numCodePoints < KN_MAX_CODE_POINT_SEQUENCE_LENGTH,
		"Trying to map a sequence of too many code points.");

	for (uint32_t i = 0; i < map->usedSequences; ++i) {
		if (CodePointSequence_Is(&map->sequences[i], codePoint, numCodePoints)) {
			return map->glyphs[i];
		}
	}
	return KN_GLYPH_INDEX_INVALID;
}

/**
 * Creates a glyph mapping for a code point sequence.
 */
bool Utf8GlyphMap_Map(Utf8GlyphMap* map, const char* codePoint, uint8_t numCodePoints, GlyphIndex glyphIndex)
{
	KN_ASSERT(map != NULL, "Cannot write a glyph to a null map.");
	KN_ASSERT(numCodePoints < KN_MAX_CODE_POINT_SEQUENCE_LENGTH,
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
		return false;
	}

	// All sequences already used.
	if (map->usedSequences + 1 == KN_MAX_GLYPH_MAP_CODE_POINT_SEQUENCES) {
		return false;
	}

	CodePointSequence_Create(&map->sequences[map->usedSequences], codePoint, numCodePoints);
	map->glyphs[map->usedSequences] = glyphIndex;
	++map->usedSequences;

	return true;
}

KN_TEST_SUITE_BEGIN("font-psf2")
	printf("Size of Utf8GlyphMap: %zu bytes\n", sizeof(Utf8GlyphMap));

	Log_Init();
	Assets_Init("assets");

	KN_TEST_UNIT("Unicode table creation") {
		Utf8GlyphMap map;
		Utf8GlyphMap_Create(&map);
	}

	KN_TEST_UNIT("Code point sequence equality") {
		CodePointSequence a;
		CodePointSequence_Create(&a, "a", 1);
		KN_TEST_ASSERT_EQ_U8(a.byteLength, 1);
		KN_TEST_ASSERT_EQ_U8(a.sequenceLength, 1);
		KN_TEST_ASSERT_TRUE(CodePointSequence_Is(&a, "a", 1));
		KN_TEST_ASSERT_FALSE(CodePointSequence_Is(&a, "b", 1));
		KN_TEST_ASSERT_FALSE(CodePointSequence_Is(&a, "ba", 2));
		KN_TEST_ASSERT_FALSE(CodePointSequence_Is(&a, "ab", 2));

		CodePointSequence b;
		CodePointSequence_Create(&b, "b", 1);
		KN_TEST_ASSERT_EQ_U8(b.byteLength, 1);
		KN_TEST_ASSERT_EQ_U8(b.sequenceLength, 1);
		KN_TEST_ASSERT_TRUE(CodePointSequence_Is(&b, "b", 1));

		CodePointSequence extended;
		CodePointSequence_Create(&extended, "\xe2\x80\xa2\xe2\x88\x99", 2);
		KN_TEST_ASSERT_EQ_U8(extended.byteLength, 6);
		KN_TEST_ASSERT_EQ_U8(extended.sequenceLength, 2);
		KN_TEST_ASSERT_TRUE(CodePointSequence_Is(&extended, "\xe2\x80\xa2\xe2\x88\x99", 2));
		KN_TEST_ASSERT_FALSE(CodePointSequence_Is(&extended, "\xe2\x80\xa2\xe2\x88\x98", 2));
	}

	KN_TEST_UNIT("Glyph mapping") {
		Utf8GlyphMap map;
		Utf8GlyphMap_Create(&map);
		Utf8GlyphMap_Map(&map, "a", 1, 5);
		KN_TEST_ASSERT_EQ_U32(5, Utf8GlyphMap_GlyphForCodePoint(&map, "a", 1));

	}

	KN_TEST_UNIT("Font loading") {
//		PathBuffer fontPath;
//		Assets_PathBufferFor("font/bizcat.psf", &fontPath);
//		FontPSF2 font;
//		ImageRGBA8 image;
//		Font_PSF2Allocate(&image, &font, fontPath.str);
//		ImageRGBA8_Free(&image);
	}

	Assets_Shutdown();
	Log_Shutdown();

KN_TEST_SUITE_END
