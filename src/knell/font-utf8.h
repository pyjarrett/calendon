#ifndef KN_FONT_UTF8_H
#define KN_FONT_UTF8_H

#include <knell/kn.h>

#define KN_MAX_UTF8_CODE_POINT_BYTE_LENGTH 4
#define KN_MAX_CODE_POINT_SEQUENCE_LENGTH 2

KN_UNIT_API uint8_t Utf8_NumBytesInCodePoint(char leadingByte);
KN_UNIT_API bool Utf8_CodePointsMatch(const char* left, const char* right);

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

KN_UNIT_API void CodePointSequence_Create(CodePointSequence* seq, const char* codePoint, uint8_t numCodePoints);
KN_UNIT_API bool CodePointSequence_Is(CodePointSequence* seq, const char* codePoint, uint8_t numCodePoints);

#endif /* KN_FONT_UTF8_H */
