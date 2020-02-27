#include "font-utf8.h"

#include <string.h>

/**
 * The number of bytes in a utf-8 char can be determined by the upper nibble
 * of the first byte.
 *
 * Number of bytes:
 * 1: 0xxx xxxx : <7
 * 2: 110x xxxx : (C|D)
 * 3: 1110 xxxx : E
 * 4: 1111 0xxx : F
 */
KN_UNIT_API uint8_t Utf8_NumBytesInCodePoint(char leadingByte)
{
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
 * Checks to see if two possibly multiple byte code points equal each other.
 */
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

/**
 * Copies a UTf-8 code point, with an interface similar to `memcpy`, or `strcpy`.
 */
KN_UNIT_API void Utf8_CodePointCopy(char* dest, const char* src)
{
	KN_ASSERT(dest != NULL, "Cannot copy to null code point.");
	KN_ASSERT(src != NULL, "Cannot copy a null code point.");
	memcpy(dest, src, Utf8_NumBytesInCodePoint(*src));
}

KN_UNIT_API void Grapheme_Create(Grapheme* seq, const char* codePoint, uint8_t numCodePoints)
{
	KN_ASSERT(seq != NULL, "Cannot create a null Grapheme.");
	KN_ASSERT(codePoint != NULL, "Cannot create a Grapheme from a null code point.");
	KN_ASSERT(numCodePoints <= KN_MAX_CODE_POINTS_IN_SEQUENCE,
		"Too many code points provided for a Grapheme: %" PRIu8,
		numCodePoints);

	const char* currentCodePoint = codePoint;
	uint8_t currentCodePointIndex = 0;
	uint8_t usedBytes = 0;
	while (currentCodePointIndex < numCodePoints) {
		Utf8_CodePointCopy(&seq->codePoints[usedBytes], currentCodePoint);
		const uint8_t bytesInCodePoint = Utf8_NumBytesInCodePoint(*currentCodePoint);
		usedBytes += bytesInCodePoint;
		currentCodePoint += bytesInCodePoint;
		++currentCodePointIndex;
	}
	seq->byteLength = usedBytes;
	seq->codePointLength = numCodePoints;
}

KN_UNIT_API bool Grapheme_Is(Grapheme* seq, const char* codePoint, uint8_t numCodePoints)
{
	KN_ASSERT(seq != NULL, "A null Grapheme is not equal to anything.");
	KN_ASSERT(codePoint != NULL, "Cannot compare a Grapheme against a NULL code point.");
	KN_ASSERT(numCodePoints < KN_MAX_CODE_POINTS_IN_SEQUENCE, "Too many code points to test for grapheme equality.");

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

KN_UNIT_API bool Grapheme_Equal(Grapheme* left, Grapheme* right)
{
	KN_ASSERT(left != NULL, "Left Grapheme is NULL.");
	KN_ASSERT(right != NULL, "Right Grapheme is NULL.");
	return Grapheme_Is(left, right->codePoints, right->codePointLength);
}

KN_UNIT_API void Grapheme_Begin(Grapheme* seq, const char* codePoint)
{
	KN_ASSERT(seq != NULL, "Cannot begin a null Grapheme.");
	KN_ASSERT(codePoint != NULL, "Cannot begin a null Grapheme with a null code point");

	seq->codePointLength = 1;
	seq->byteLength = Utf8_NumBytesInCodePoint(codePoint);
	Utf8_CodePointCopy(&seq->codePoints[0], codePoint);
}

KN_UNIT_API bool Grapheme_AddCodePoint(Grapheme* seq, const char* codePoint)
{
	KN_ASSERT(seq != NULL, "Cannot add to a null Grapheme.");
	KN_ASSERT(codePoint != NULL, "Cannot add a null code point to a Grapheme.");

	if (seq->codePointLength == KN_MAX_CODE_POINTS_IN_SEQUENCE) {
		return false;
	}

	KN_ASSERT(seq->byteLength + Utf8_NumBytesInCodePoint(codePoint)
			  < KN_MAX_BYTES_IN_GRAPHEME, "Grapheme will exceed its capacity");

	Utf8_CodePointCopy(&seq->codePoints[seq->byteLength], codePoint);
	++seq->codePointLength;
	seq->byteLength += Utf8_NumBytesInCodePoint(*codePoint);

	return false;
}