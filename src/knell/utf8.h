#ifndef KN_FONT_UTF8_H
#define KN_FONT_UTF8_H

/*
 * Functions and types related to supporting UTF-8.
 *
 * Working with UTF-8, from dealing with individual code points to working with
 * grapheme clusters (referred to simply as "graphemes" within Knell).
 *
 * ASCII is represented directly as-in within UTF-8, and this advantage is more
 * than worth dealing with a variable length character encoding scheme.  Knell
 * does not worry about dealing with UTF-16, or other formats; users must
 * convert their content to UTF-8 for Knell, and Knell must be capable of
 * determining if content is valid UTF-8.  This prevents any strange
 * ambiguities within the system, in which you don't know exactly which
 * encoding of a string you are dealing with at the time.  In Knell, you are
 * either dealing with UTF-8, or with ASCII, which coincidentally, is UTF-8.
 *
 * Usage of `uint8_t` over `char`: It is not specified where `char` is signed
 * or unsigned.  To eliminate any ambiguities, all UTF-8 strings within Knell
 * should be treated as `uint8_t`, and the interfaces are written as such.
 *
 * "CodePoint" is the Knell term for a series of UTF-8 encoded bytes which
 * represent a single UTF code point.
 */

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

/**
 * Not all byte sequences are valid in UTF-8.  Always illegal byte sequences
 * get the symbolic value of false, and possibly good, and good values get
 * values which will evaluate to true.
 */
typedef enum {
	Utf8ByteValidityIllegal = 0,
	Utf8ByteValidityValid = 1,
	Utf8ByteValidityPossible = 2
} Utf8ByteValidity;

KN_TEST_API Utf8ByteValidity Utf8_IsValidByte(uint8_t codeUnit);
KN_TEST_API bool Utf8_IsLeadingByte(uint8_t codeUnit);
KN_TEST_API bool Utf8_IsStringValid(const uint8_t* str);

KN_TEST_API uint8_t Utf8_NumBytesInCodePoint(uint8_t leadingByte);
KN_TEST_API bool Utf8_CodePointsMatch(const uint8_t* left, const uint8_t* right);
KN_TEST_API void Utf8_CodePointCopy(uint8_t* dest, const uint8_t* src);

KN_TEST_API size_t Utf8_StringLength(const uint8_t* str);
KN_TEST_API const uint8_t* Utf8_StringNext(const uint8_t* str);
KN_TEST_API bool Utf8_StringEqual(const uint8_t* left, const uint8_t* right);

/**
 * A sequence of code points which represents a single glyph.  The proper term
 * is "grapheme cluster" but "grapheme" is used for brevity.
 *
 * An example would be an accent applied to an e.
 */
typedef struct {
	// Add an additional space for a null byte so graphemes can be written as
	// null-terminated strings.
	uint8_t codePoints[KN_MAX_BYTES_IN_GRAPHEME + 1];
	uint8_t codePointLength;
	uint32_t byteLength;
} Grapheme;

KN_TEST_API void Grapheme_Create(Grapheme* seq, const uint8_t* codePoint, uint8_t numCodePoints);
KN_TEST_API bool Grapheme_EqualsCodePoints(Grapheme* seq, const uint8_t* codePoint, uint8_t numCodePoints);
KN_TEST_API bool Grapheme_EqualsGrapheme(Grapheme* left, Grapheme* right);

KN_TEST_API void Grapheme_Begin(Grapheme* seq);
KN_TEST_API bool Grapheme_AddCodePoint(Grapheme*, const uint8_t* codePoint);

#if KN_DEBUG
void Grapheme_Print(Grapheme* g, FILE* stream);
#endif

#endif /* KN_FONT_UTF8_H */
