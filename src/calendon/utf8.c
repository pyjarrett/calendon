#include "utf8.h"

#include <string.h>

/**
 * Note all bytes are valid in UTF-8.  This returns a non-zero value for a good
 * or potentially valid byte, so this value can work in boolean expressions.
 */
CN_TEST_API CnUtf8ByteValidity cnUtf8_IsValidByte(uint8_t codeUnit)
{
	/*
	 * E0, F0: could start overlong sequences
	 * ED, F4
	 */
	switch (codeUnit) {
		case 0xE0:
		case 0xF0:
		case 0xED:
		case 0xF4:
			return CnUtf8ByteValidityPossible;
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
			return CnUtf8ByteValidityIllegal;
		default:
			return CnUtf8ByteValidityValid;
	}
}

/**
 * Only specific bytes may be the leading byte in a code point.
 */
CN_TEST_API bool cnUtf8_IsLeadingByte(uint8_t codeUnit)
{
	CN_ASSERT(cnUtf8_IsValidByte(codeUnit), "Invalid code byte: %X", codeUnit);

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
CN_TEST_API bool cnUtf8_IsStringValid(const uint8_t* str)
{
	// Null strings are not valid.
	if (!str) return false;

	const size_t length = strlen((const char*)str);
	for (size_t i = 0; i < length; ++i) {
		/*
		 * Check every byte to ensure random invalid bytes weren't inserted
		 * within the string.
		 */
		if (!cnUtf8_IsValidByte(str[i])) {
			return false;
		}
	}

	/*
	 * Verify the string ends at a null byte, and also that the sequence of
	 * following every code point also arrives at the same null byte.
	 */
	const uint8_t* cursor = str;
	while (*cursor) {
		/*
		 * We're hopping byte code points, so every hop should be a code point.
		 */
		if (!cnUtf8_IsLeadingByte(*cursor)) {
			return false;
		}

		/*
		 * Leading bytes shouldn't appear in a sequence unless a leading byte
		 * has no continuation bytes.
		 */
		const uint8_t numBytesInCodePoint = cnUtf8_NumBytesInCodePoint(*cursor);
		for (uint8_t i = 1; i < numBytesInCodePoint; ++i) {
			if (cnUtf8_IsLeadingByte(cursor[i])) {
				return false;
			}
		}
		cursor = cnUtf8_StringNext(cursor);
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
CN_TEST_API uint8_t cnUtf8_NumBytesInCodePoint(uint8_t leadingByte)
{
	CN_ASSERT(cnUtf8_IsValidByte(leadingByte),
		"Unknown number of bytes for illegal byte %X", leadingByte);
	const uint8_t b = leadingByte & 0xF0;
	switch (b) {
		case 0xF0: return 4;
		case 0xE0: return 3;
		case 0xD0: // fallthrough
		case 0xC0:
			return 2;
		default:
			CN_ASSERT((b & 0xC0) != 0x80, "Cannot count bytes in a non-leading "
				"byte: %X", leadingByte);
			return 1;
	}
}

/**
 * Checks to see if two possibly multiple byte code points equal each other.
 */
CN_TEST_API bool cnUtf8_CodePointsMatch(const uint8_t* left, const uint8_t* right)
{
	CN_ASSERT(left != NULL, "Cannot check a null code point (left-side)");
	CN_ASSERT(right != NULL, "Cannot check a null code point (right-side)");

	uint8_t leftLength = cnUtf8_NumBytesInCodePoint(*left);
	uint8_t rightLength = cnUtf8_NumBytesInCodePoint(*right);

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
CN_TEST_API void cnUtf8_CodePointCopy(uint8_t* dest, const uint8_t* src)
{
	CN_ASSERT(dest != NULL, "Cannot copy to null code point.");
	CN_ASSERT(src != NULL, "Cannot copy a null code point.");
	memcpy(dest, src, cnUtf8_NumBytesInCodePoint(*src));
}

CN_TEST_API size_t cnUtf8_StringLength(const uint8_t* str)
{
	CN_ASSERT(str != NULL, "Cannot get length of a null string");

	size_t numCodePoints = 0;
	const uint8_t* cursor = str;
	while (*cursor) {
		++numCodePoints;
		cursor = cnUtf8_StringNext(cursor);
	}
	CN_ASSERT(cursor == (str + strlen((const char*)str)),
		"Invalid continuation byte at end of string.");
	return numCodePoints;
}

CN_TEST_API const uint8_t* cnUtf8_StringNext(const uint8_t* str)
{
	CN_ASSERT(str != NULL, "Cannot get next code point in a null string.");
	const uint32_t numBytesInCodePoint = cnUtf8_NumBytesInCodePoint(*str);
//	for (uint32_t i = 0; i < numBytesInCodePoint; ++i) {
//		CN_ASSERT(*(str + i) != 0, "Invalid continuation byte");
//	}
	return str + numBytesInCodePoint;
}

CN_TEST_API bool cnUtf8_StringEqual(const uint8_t* left, const uint8_t* right)
{
	if (cnUtf8_StringLength(left) != cnUtf8_StringLength(right)) {
		return false;
	}
	return strcmp((const char*)left, (const char*)right) == 0;
}

/**
 * Initialize a grapheme with an initial code point.  All other possible bytes
 * within the grapheme are zeroed.
 */
CN_TEST_API void cnGrapheme_Set(CnGrapheme* seq, const uint8_t* codePoint, uint8_t numCodePoints)
{
	CN_ASSERT(seq != NULL, "Cannot create a null CnGrapheme.");
	CN_ASSERT(codePoint != NULL, "Cannot create a CnGrapheme from a null code point.");
	CN_ASSERT(numCodePoints <= CN_MAX_CODE_POINTS_IN_GRAPHEME,
		"Too many code points provided for a CnGrapheme: %" PRIu8,
		numCodePoints);

	memset(&seq->codePoints[0], 0, CN_MAX_BYTES_IN_GRAPHEME);
	const uint8_t* currentCodePoint = codePoint;
	uint8_t currentCodePointIndex = 0;
	uint8_t usedBytes = 0;
	while (currentCodePointIndex < numCodePoints) {
		cnUtf8_CodePointCopy(&seq->codePoints[usedBytes], currentCodePoint);
		usedBytes += cnUtf8_NumBytesInCodePoint(*currentCodePoint);
		currentCodePoint = cnUtf8_StringNext(currentCodePoint);
		++currentCodePointIndex;
	}
	seq->byteLength = usedBytes;
	seq->codePointLength = numCodePoints;
}

/**
 * Compares a grapheme, which is a series of code points, against a series of
 * code points.
 */
CN_TEST_API bool cnGrapheme_EqualsCodePoints(CnGrapheme* seq, const uint8_t* codePoint, uint8_t numCodePoints)
{
	CN_ASSERT(seq != NULL, "A null CnGrapheme is not equal to anything.");
	CN_ASSERT(codePoint != NULL, "Cannot compare a CnGrapheme against a NULL code point.");
	CN_ASSERT(numCodePoints <= CN_MAX_CODE_POINTS_IN_GRAPHEME, "Too many code points %" PRIu8
		"to test for grapheme equality.", numCodePoints);

	const uint8_t* currentCodePoint = codePoint;
	uint8_t currentCodePointIndex = 0;
	uint8_t usedBytes = 0;
	while (currentCodePointIndex < numCodePoints) {
		const uint8_t bytesInCodePoint = cnUtf8_NumBytesInCodePoint(*currentCodePoint);
		for (uint8_t i = 0; i < bytesInCodePoint; ++i) {
			if (seq->codePoints[usedBytes++] != currentCodePoint[i]) {
				return false;
			}
		}
		currentCodePoint = cnUtf8_StringNext(currentCodePoint);
		++currentCodePointIndex;
	}
	return true;
}

/**
 * Compares one grapheme against another grapheme.
 */
CN_TEST_API bool cnGrapheme_EqualsGrapheme(CnGrapheme* left, CnGrapheme* right)
{
	CN_ASSERT(left != NULL, "Left CnGrapheme is NULL.");
	CN_ASSERT(right != NULL, "Right CnGrapheme is NULL.");
	return cnGrapheme_EqualsCodePoints(left, right->codePoints, right->codePointLength);
}

/**
 * Initializes an empty grapheme.
 */
CN_TEST_API void cnGrapheme_Begin(CnGrapheme* seq)
{
	CN_ASSERT(seq != NULL, "Cannot begin a null CnGrapheme.");
	seq->codePointLength = 0;
	seq->byteLength = 0;
	memset(&seq->codePoints[0], 0, CN_MAX_BYTES_IN_GRAPHEME);
}

CN_TEST_API bool cnGrapheme_AddCodePoint(CnGrapheme* seq, const uint8_t* codePoint)
{
	CN_ASSERT(seq != NULL, "Cannot add to a null CnGrapheme.");
	CN_ASSERT(codePoint != NULL, "Cannot add a null code point to a CnGrapheme.");

	if (seq->codePointLength == CN_MAX_CODE_POINTS_IN_GRAPHEME) {
		return false;
	}

	CN_ASSERT(seq->byteLength + cnUtf8_NumBytesInCodePoint(*codePoint)
			  < CN_MAX_BYTES_IN_GRAPHEME, "CnGrapheme will exceed its capacity");

	cnUtf8_CodePointCopy(&seq->codePoints[seq->byteLength], codePoint);
	++seq->codePointLength;
	seq->byteLength += cnUtf8_NumBytesInCodePoint(*codePoint);

	return false;
}

#if CN_DEBUG
void CnGrapheme_Print(CnGrapheme* g, FILE* stream)
{
	CN_ASSERT(g != NULL, "Cannot print a null grapheme.");
	CN_ASSERT(stream != NULL, "Cannot print a grapheme to a null stream.");
	fprintf(stream, "[%8x %8x %8x %8x] %s",
		g->codePoints[0],
		g->codePoints[1],
		g->codePoints[2],
		g->codePoints[3],
		&g->codePoints[0]);
}
#endif
