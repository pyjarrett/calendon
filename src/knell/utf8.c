#include "utf8.h"

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
KN_TEST_API uint8_t Utf8_NumBytesInCodePoint(uint8_t leadingByte)
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
KN_TEST_API bool Utf8_CodePointsMatch(const uint8_t* left, const uint8_t* right)
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
 * Copies a UTF-8 code point, with an interface similar to `memcpy`, or `strcpy`.
 */
KN_TEST_API void Utf8_CodePointCopy(uint8_t* dest, const uint8_t* src)
{
	KN_ASSERT(dest != NULL, "Cannot copy to null code point.");
	KN_ASSERT(src != NULL, "Cannot copy a null code point.");
	memcpy(dest, src, Utf8_NumBytesInCodePoint(*src));
}

KN_TEST_API size_t Utf8_StringLength(const uint8_t* str)
{
	KN_ASSERT(str != NULL, "Cannot get length of a null string");

	size_t numCodePoints = 0;
	const uint8_t* cursor = str;
	while (*cursor) {
		++numCodePoints;
		cursor = Utf8_StringNext(cursor);
	}
	return numCodePoints;
}

KN_TEST_API const uint8_t* Utf8_StringNext(const uint8_t* str)
{
	KN_ASSERT(str != NULL, "Cannot get next code point in a null string.");
	return str + Utf8_NumBytesInCodePoint(*str);
}

KN_TEST_API bool Utf_StringEqual(const uint8_t* left, const uint8_t* right)
{
	return strcmp((const char*)left, (const char*)right) == 0;
}

KN_TEST_API void Grapheme_Create(Grapheme* seq, const uint8_t* codePoint, uint8_t numCodePoints)
{
	KN_ASSERT(seq != NULL, "Cannot create a null Grapheme.");
	KN_ASSERT(codePoint != NULL, "Cannot create a Grapheme from a null code point.");
	KN_ASSERT(numCodePoints <= KN_MAX_CODE_POINTS_IN_GRAPHEME,
		"Too many code points provided for a Grapheme: %" PRIu8,
		numCodePoints);

	memset(&seq->codePoints[0], 0, KN_MAX_BYTES_IN_GRAPHEME);
	const uint8_t* currentCodePoint = codePoint;
	uint8_t currentCodePointIndex = 0;
	uint8_t usedBytes = 0;
	while (currentCodePointIndex < numCodePoints) {
		Utf8_CodePointCopy(&seq->codePoints[usedBytes], currentCodePoint);
		usedBytes += Utf8_NumBytesInCodePoint(*currentCodePoint);
		currentCodePoint = Utf8_StringNext(currentCodePoint);
		++currentCodePointIndex;
	}
	seq->byteLength = usedBytes;
	seq->codePointLength = numCodePoints;
}

KN_TEST_API bool Grapheme_EqualsCodePoints(Grapheme* seq, const uint8_t* codePoint, uint8_t numCodePoints)
{
	KN_ASSERT(seq != NULL, "A null Grapheme is not equal to anything.");
	KN_ASSERT(codePoint != NULL, "Cannot compare a Grapheme against a NULL code point.");
	KN_ASSERT(numCodePoints <= KN_MAX_CODE_POINTS_IN_GRAPHEME, "Too many code points %" PRIu8
		"to test for grapheme equality.", numCodePoints);

	const uint8_t* currentCodePoint = codePoint;
	uint8_t currentCodePointIndex = 0;
	uint8_t usedBytes = 0;
	while (currentCodePointIndex < numCodePoints) {
		const uint8_t bytesInCodePoint = Utf8_NumBytesInCodePoint(*currentCodePoint);
		for (uint8_t i = 0; i < bytesInCodePoint; ++i) {
			if (seq->codePoints[usedBytes++] != currentCodePoint[i]) {
				return false;
			}
		}
		currentCodePoint = Utf8_StringNext(currentCodePoint);
		++currentCodePointIndex;
	}
	return true;
}

KN_TEST_API bool Grapheme_EqualsGrapheme(Grapheme* left, Grapheme* right)
{
	KN_ASSERT(left != NULL, "Left Grapheme is NULL.");
	KN_ASSERT(right != NULL, "Right Grapheme is NULL.");
	return Grapheme_EqualsCodePoints(left, right->codePoints, right->codePointLength);
}

KN_TEST_API void Grapheme_Begin(Grapheme* seq)
{
	KN_ASSERT(seq != NULL, "Cannot begin a null Grapheme.");
	seq->codePointLength = 0;
	seq->byteLength = 0;
	memset(&seq->codePoints[0], 0, KN_MAX_BYTES_IN_GRAPHEME);
}

KN_TEST_API bool Grapheme_AddCodePoint(Grapheme* seq, const uint8_t* codePoint)
{
	KN_ASSERT(seq != NULL, "Cannot add to a null Grapheme.");
	KN_ASSERT(codePoint != NULL, "Cannot add a null code point to a Grapheme.");

	if (seq->codePointLength == KN_MAX_CODE_POINTS_IN_GRAPHEME) {
		return false;
	}

	KN_ASSERT(seq->byteLength + Utf8_NumBytesInCodePoint(*codePoint)
			  < KN_MAX_BYTES_IN_GRAPHEME, "Grapheme will exceed its capacity");

	Utf8_CodePointCopy(&seq->codePoints[seq->byteLength], codePoint);
	++seq->codePointLength;
	seq->byteLength += Utf8_NumBytesInCodePoint(*codePoint);

	return false;
}

#if KN_DEBUG
void Grapheme_Print(Grapheme* g, FILE* f)
{
	KN_ASSERT(g != NULL, "Cannot print a null grapheme.");
	KN_ASSERT(f != NULL, "Cannot print a grapheme to a null stream.");
	fprintf(f, "[%8x %8x %8x %8x] %s",
		g->codePoints[0],
		g->codePoints[1],
		g->codePoints[2],
		g->codePoints[3],
		&g->codePoints[0]);
}
#endif
