#include "font-utf8.h"

KN_UNIT_API uint8_t Utf8_NumBytesInCodePoint(char leadingByte)
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

// https://fasterthanli.me/blog/2020/working-with-strings-in-rust/
KN_UNIT_API bool Utf8_CodePointsMatch(const char* left, const char* right)
{
	KN_ASSERT(left != NULL, "Cannot check a null code point (left-side)");
	KN_ASSERT(right != NULL, "Cannot check a null code point (right-side)");

	uint8_t leftLength = Utf8_NumBytesInCodePoint(*left);
	uint8_t rightLength = Utf8_NumBytesInCodePoint(*right);

	if (leftLength != rightLength) {
		return false;
	}

	for (uint8_t i=0; i < leftLength; ++i) {
		if (left[i] != right[i]) {
			return false;
		}
	}
	return true;
}

KN_UNIT_API void CodePointSequence_Create(CodePointSequence* seq, const char* codePoint, uint8_t numCodePoints)
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
		const uint8_t bytesInCodePoint = Utf8_NumBytesInCodePoint(*currentCodePoint);
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

KN_UNIT_API bool CodePointSequence_Is(CodePointSequence* seq, const char* codePoint, uint8_t numCodePoints)
{
	const char* currentCodePoint = codePoint;
	uint8_t currentCodePointIndex = 0;
	uint8_t usedBytes = 0;
	while (currentCodePointIndex < numCodePoints) {
		const uint8_t bytesInCodePoint = Utf8_NumBytesInCodePoint(*currentCodePoint);
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
