#include "utf8.h"

#include <string.h>

/**
 * Not all bytes are valid in UTF-8.  This returns a non-zero value for a good
 * or potentially valid byte, so this value can work in boolean expressions.
 */
KN_TEST_API Utf8ByteValidity Utf8_IsValidByte(KNchar aByte)
{
	/*
	 * E0, F0: could start overlong sequences
	 * ED, F4
	 */
	switch (aByte) {
		case 0xE0:
		case 0xF0:
		case 0xED:
		case 0xF4:
			return Utf8ByteValidityPossible;
	// Disallowed sequences of 2 byte, 7-bit ASCII character encodings.
		case 0xC0:
		case 0xC1:
	// Sequences which would be 4 bytes or longer.
		case 0xF5:
		case 0xF6:
		case 0xF7:
		case 0xF8:
		case 0xF9:
		case 0xFA:
		case 0xFB:
		case 0xFC:
		case 0xFD:
	// Not defined for any purpose in UTF-8.
		case 0xFE:
		case 0xFF:
			return Utf8ByteValidityIllegal;
		default:
			return Utf8ByteValidityValid;
	}
}

/**
 * Only specific bytes may be the leading byte in a code point.
 */
KN_TEST_API bool Utf8_IsLeadingByte(KNchar codeUnit)
{
	KN_ASSERT(Utf8_IsValidByte(codeUnit), "Invalid code byte: %X", codeUnit);

	// 1 leading byte
	if ((codeUnit & 0x80) == 0) return true;

	// 2 leading bytes
	if ((codeUnit & 0xE0) == 0xC0) return true;

	// 3 leading bytes
	if ((codeUnit & 0xF0) == 0xE0) return true;

	// 4 leading bytes
	if ((codeUnit & 0xF8) == 0xF0) return true;

	return false;
}

/**
 * Testing for UTF-8 string validity is somewhat complicated.  Implementing as
 * many rules as I understand at the moment.
 */
// TODO: Security risk, should be researched further and reviewed again.
KN_TEST_API bool Utf8_IsStringValid(const KNchar* str)
{
	// Null strings are not valid.
	if (!str) return false;

	const size_t length = strlen((const char*)str);
	for (size_t i = 0; i < length; ++i) {
		/*
		 * Check every byte to ensure random invalid bytes weren't inserted
		 * within the string.
		 */
		if (!Utf8_IsValidByte(str[i])) {
			return false;
		}
	}

	/*
	 * Verify the string ends at a null byte, and also that the sequence of
	 * following every code point also arrives at the same null byte.
	 */
	const KNchar* cursor = str;
	while (*cursor) {
		/*
		 * We're hopping byte code points, so every hop should be a code point.
		 */
		if (!Utf8_IsLeadingByte(*cursor)) {
			return false;
		}

		/*
		 * Leading bytes shouldn't appear in a sequence unless a leading byte
		 * has no continuation bytes.
		 */
		const uint8_t numBytesInCodePoint = Utf8_NumBytesInCodePoint(*cursor);
		for (uint8_t i = 1; i < numBytesInCodePoint; ++i) {
			if (Utf8_IsLeadingByte(cursor[i])) {
				return false;
			}
		}
		cursor = Utf8_StringNext(cursor);
	}
	return cursor == (str + length);
}

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
KN_TEST_API uint8_t Utf8_NumBytesInCodePoint(KNchar leadingByte)
{
	KN_ASSERT(Utf8_IsValidByte(leadingByte),
		"Unknown number of bytes for illegal byte %X", leadingByte);
	const KNchar b = leadingByte & 0xF0;
	switch (b) {
		case 0xF0: return 4;
		case 0xE0: return 3;
		case 0xD0: // fallthrough
		case 0xC0:
			return 2;
		default:
			KN_ASSERT((b & 0xC0) != 0x80, "Cannot count bytes in a non-leading "
				"byte: %X", leadingByte);
			return 1;
	}
}

/**
 * Checks to see if two possibly multiple byte code points equal each other.
 */
KN_TEST_API bool Utf8_CodePointsMatch(const KNchar* left, const KNchar* right)
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
KN_TEST_API void Utf8_CodePointCopy(KNchar* dest, const KNchar* src)
{
	KN_ASSERT(dest != NULL, "Cannot copy to null code point.");
	KN_ASSERT(src != NULL, "Cannot copy a null code point.");
	memcpy(dest, src, Utf8_NumBytesInCodePoint(*src));
}

KN_TEST_API size_t Utf8_StringLength(const KNchar* str)
{
	KN_ASSERT(str != NULL, "Cannot get length of a null string");

	size_t numCodePoints = 0;
	const KNchar* cursor = str;
	while (*cursor) {
		++numCodePoints;
		cursor = Utf8_StringNext(cursor);
	}
	KN_ASSERT(cursor == (str + strlen((const char*)str)),
		"Invalid continuation byte at end of string.");
	return numCodePoints;
}

KN_TEST_API const KNchar* Utf8_StringNext(const KNchar* str)
{
	KN_ASSERT(str != NULL, "Cannot get next code point in a null string.");
	const uint32_t numBytesInCodePoint = Utf8_NumBytesInCodePoint(*str);
//	for (uint32_t i = 0; i < numBytesInCodePoint; ++i) {
//		KN_ASSERT(*(str + i) != 0, "Invalid continuation byte");
//	}
	return str + numBytesInCodePoint;
}

KN_TEST_API bool Utf8_StringEqual(const KNchar* left, const KNchar* right)
{
	if (Utf8_StringLength(left) != Utf8_StringLength(right)) {
		return false;
	}
	return strcmp((const char*)left, (const char*)right) == 0;
}

/**
 * Initialize a grapheme with an initial code point.  All other possible bytes
 * within the grapheme are zeroed.
 */
KN_TEST_API void Grapheme_Set(Grapheme* seq, const KNchar* codePoint, uint8_t numCodePoints)
{
	KN_ASSERT(seq != NULL, "Cannot create a null Grapheme.");
	KN_ASSERT(codePoint != NULL, "Cannot create a Grapheme from a null code point.");
	KN_ASSERT(numCodePoints <= KN_MAX_CODE_POINTS_IN_GRAPHEME,
		"Too many code points provided for a Grapheme: %" PRIu8,
		numCodePoints);

	memset(&seq->codePoints[0], 0, KN_MAX_BYTES_IN_GRAPHEME);
	const KNchar* currentCodePoint = codePoint;
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

/**
 * Compares a grapheme, which is a series of code points, against a series of
 * code points.
 */
KN_TEST_API bool Grapheme_EqualsCodePoints(Grapheme* seq, const KNchar* codePoint, uint8_t numCodePoints)
{
	KN_ASSERT(seq != NULL, "A null Grapheme is not equal to anything.");
	KN_ASSERT(codePoint != NULL, "Cannot compare a Grapheme against a NULL code point.");
	KN_ASSERT(numCodePoints <= KN_MAX_CODE_POINTS_IN_GRAPHEME, "Too many code points %" PRIu8
		"to test for grapheme equality.", numCodePoints);

	const KNchar* currentCodePoint = codePoint;
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

/**
 * Compares one grapheme against another grapheme.
 */
KN_TEST_API bool Grapheme_EqualsGrapheme(Grapheme* left, Grapheme* right)
{
	KN_ASSERT(left != NULL, "Left Grapheme is NULL.");
	KN_ASSERT(right != NULL, "Right Grapheme is NULL.");
	return Grapheme_EqualsCodePoints(left, right->codePoints, right->codePointLength);
}

/**
 * Initializes an empty grapheme.
 */
KN_TEST_API void Grapheme_Begin(Grapheme* seq)
{
	KN_ASSERT(seq != NULL, "Cannot begin a null Grapheme.");
	seq->codePointLength = 0;
	seq->byteLength = 0;
	memset(&seq->codePoints[0], 0, KN_MAX_BYTES_IN_GRAPHEME);
}

KN_TEST_API bool Grapheme_AddCodePoint(Grapheme* seq, const KNchar* codePoint)
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
