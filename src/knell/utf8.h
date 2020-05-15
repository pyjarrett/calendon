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
 * Usage of `KNchar` over `char`: It is not specified where `char` is signed
 * or unsigned.  To eliminate any ambiguities, all UTF-8 strings within Knell
 * should be treated as `KNchar`, and the interfaces are written as such.
 * Using `uint8_t` seems a more accurate solution, but using `KNchar` ensures
 * better semantics, while also silencing clang/gcc errors.
 *
 * "CodePoint" is the Knell term for a series of UTF-8 encoded bytes which
 * represent a single UTF code point.
 */

#include <knell/kn.h>

typedef signed char KNchar;

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

KN_TEST_API Utf8ByteValidity Utf8_IsValidByte(KNchar codeUnit);
KN_TEST_API bool Utf8_IsLeadingByte(KNchar codeUnit);
KN_TEST_API bool Utf8_IsStringValid(const KNchar* str);

KN_TEST_API uint8_t Utf8_NumBytesInCodePoint(KNchar leadingByte);
KN_TEST_API bool Utf8_CodePointsMatch(const KNchar* left, const KNchar* right);
KN_TEST_API void Utf8_CodePointCopy(KNchar* dest, const KNchar* src);

KN_TEST_API size_t Utf8_StringLength(const KNchar* str);
KN_TEST_API const KNchar* Utf8_StringNext(const KNchar* str);
KN_TEST_API bool Utf8_StringEqual(const KNchar* left, const KNchar* right);

/**
 * A sequence of code points which represents a single glyph.  The proper term
 * is "grapheme cluster" but "grapheme" is used for brevity.
 *
 * An example would be an accent applied to an e.
 */
typedef struct {
	// Add an additional space for a null byte so graphemes can be written as
	// null-terminated strings.
	KNchar codePoints[KN_MAX_BYTES_IN_GRAPHEME + 1];
	uint8_t codePointLength;
	uint32_t byteLength;
} Grapheme;

KN_TEST_API void Grapheme_Set(Grapheme* seq, const KNchar* codePoint, uint8_t numCodePoints);
KN_TEST_API bool Grapheme_EqualsCodePoints(Grapheme* seq, const KNchar* codePoint, uint8_t numCodePoints);
KN_TEST_API bool Grapheme_EqualsGrapheme(Grapheme* left, Grapheme* right);

KN_TEST_API void Grapheme_Begin(Grapheme* seq);
KN_TEST_API bool Grapheme_AddCodePoint(Grapheme*, const KNchar* codePoint);

#if KN_DEBUG
void Grapheme_Print(Grapheme* g, FILE* stream);
#endif

#endif /* KN_FONT_UTF8_H */
