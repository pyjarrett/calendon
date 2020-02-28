#ifndef KN_FONT_UTF8_H
#define KN_FONT_UTF8_H

#include <knell/kn.h>

/**
 * The maximum number of bytes to encode a UTF-8 code point is 4.
 */
#define KN_MAX_UTF8_CODE_POINT_BYTE_LENGTH 4

/**
 * There is technically no limit on the number of code points which may form a
 * grapheme.  The approach here is to set a reasonable limit to allow for
 * development to continue.
 */
#define KN_MAX_CODE_POINTS_IN_GRAPHEME 3
#define KN_MAX_BYTES_IN_GRAPHEME (KN_MAX_CODE_POINTS_IN_GRAPHEME * KN_MAX_UTF8_CODE_POINT_BYTE_LENGTH)

KN_UNIT_API uint8_t Utf8_NumBytesInCodePoint(uint8_t leadingByte);
KN_UNIT_API bool Utf8_CodePointsMatch(const uint8_t* left, const uint8_t* right);
KN_UNIT_API void Utf8_CodePointCopy(uint8_t* dest, const uint8_t* src);

/**
 * A sequence of code points which represents\ a single glyph.  The proper term
 * is "grapheme cluster" but using grapheme for brevity.
 *
 * An example would be an accent applied to an e.
 */
typedef struct {
	// Add an additional space for anull byte so graphemes can be written as
	// null-terminated strings.
	uint8_t codePoints[KN_MAX_BYTES_IN_GRAPHEME + 1];
	uint8_t codePointLength;
	uint32_t byteLength;
} Grapheme;

KN_UNIT_API void Grapheme_Create(Grapheme* seq, const uint8_t* codePoint, uint8_t numCodePoints);
KN_UNIT_API bool Grapheme_Is(Grapheme* seq, const uint8_t* codePoint, uint8_t numCodePoints);
KN_UNIT_API bool Grapheme_Equal(Grapheme* left, Grapheme* right);

KN_UNIT_API void Grapheme_Begin(Grapheme* seq);
KN_UNIT_API bool Grapheme_AddCodePoint(Grapheme*, const uint8_t* codePoint);

#if KN_DEBUG
void Grapheme_Print(Grapheme* g, FILE* stream);
#endif

#endif /* KN_FONT_UTF8_H */
