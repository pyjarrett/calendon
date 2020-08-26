#include "string.h"

#include <calendon/cn.h>
#include <string.h>

bool cnString_FitsWithNull(const char* str, const size_t bytes)
{
	CN_ASSERT_PTR(str);

	CN_ASSERT(bytes != 0, "Strings must be at least 1 byte long.");
	CN_ASSERT(bytes < SIZE_MAX, "Strings must be less than %zu bytes", SIZE_MAX);

	return strlen(str) + 1 <= bytes;
}

/**
 * Counts the number of bytes in a null terminated string up to a given number
 * of max bytes.
 */
bool cnString_StorageRequired(const char* str, const size_t maxBytes, size_t* length)
{
	CN_ASSERT_PTR(str);
	CN_ASSERT_PTR(length);

	CN_ASSERT(maxBytes != 0, "Strings must be at least 1 byte long.");
	CN_ASSERT(maxBytes < SIZE_MAX, "Strings must be less than %zu bytes", SIZE_MAX);

	const char* ptr = str;
	size_t bytes = 0;
	while (bytes < maxBytes && *ptr) {
		++bytes;
		++ptr;
	}
	*length = bytes + 1;
	return bytes + 1 <= maxBytes;
}

bool cnString_NumCharacterBytes(const char* str, const size_t maxCharBytes, size_t* length)
{
	const bool succeeded = cnString_StorageRequired(str, maxCharBytes + 1, length);
	*length = (*length) - 1;
	return succeeded;
}

bool cnString_Equal(const char* left, const char* right, const size_t maxCharBytes)
{
	CN_ASSERT_PTR(left);
	CN_ASSERT_PTR(right);

	size_t numLeftBytes, numRightBytes;
	const bool leftTerminated = cnString_NumCharacterBytes(left, maxCharBytes, &numLeftBytes);
	const bool rightTerminated = cnString_NumCharacterBytes(right, maxCharBytes, &numRightBytes);

	if (!leftTerminated) {
		CN_ASSERT(false, "Unterminated string.");
		return false;
	}

	if (!rightTerminated) {
		CN_ASSERT(false, "Unterminated string.");
	}

	return strncmp(left, right, maxCharBytes) == 0;
}

/**
 * Copies at most a specific number of characters from the source to the
 * destination.
 *
 * Returns false and does not modify the destination if the source does not
 * terminate after that number of characters.
 */
bool cnString_Copy(char* destination, const char* source, const size_t maxCharBytes)
{
	CN_ASSERT_PTR(destination);
	CN_ASSERT_PTR(source);

	size_t numSourceBytes;

	const bool sourceTerminated = cnString_NumCharacterBytes(source, maxCharBytes, &numSourceBytes);
	if (!sourceTerminated) {
		CN_ASSERT(false, "Unterminated source string in copy.");
		return false;
	}

	const char* s = source;
	char* d = destination;
	while(*s) {
		*d = *s;
		++s;
		++d;
	}
	*d = 0;

	return true;
}

bool cnString_HasPrefix(const char* str, const size_t maxCharBytes, const char* prefix)
{
	CN_ASSERT_PTR(str);
	CN_ASSERT_PTR(prefix);

	size_t numStrBytes, numPrefixBytes;
	const bool strTerminated = cnString_NumCharacterBytes(str, maxCharBytes, &numStrBytes);
	const bool prefixTerminated = cnString_NumCharacterBytes(prefix, maxCharBytes, &numPrefixBytes);

	if (!strTerminated) {
		CN_ASSERT(false, "Unterminated string.");
		return false;
	}

	if (!prefixTerminated) {
		CN_ASSERT(false, "Unterminated prefix.");
	}

	if (numStrBytes < numPrefixBytes) {
		return false;
	}

	size_t bytesMatched = 0;
	const char* s = str;
	const char* p = prefix;
	while (bytesMatched < numPrefixBytes && *s == *p) {
		++s;
		++p;
		++bytesMatched;
	}

	return bytesMatched == numPrefixBytes;
}

bool cnString_FirstIndexOfChar(const char* str, const size_t maxBytes, char ch, size_t* index)
{
	CN_ASSERT_PTR(str);
	CN_ASSERT_PTR(index);
	CN_ASSERT(maxBytes != SIZE_MAX, "Max bytes must be < SIZE_MAX");

	size_t strLength;
	CN_ASSERT(cnString_NumCharacterBytes(str, maxBytes, &strLength),
		"Entire string might not be checked for character.");

	size_t currentIndex = 0;
	const char* ptr = str;
	while (currentIndex < strLength) {
		if (*ptr == ch) {
			*index = currentIndex;
			return true;
		}
		++ptr;
		++currentIndex;
	}

	CN_ASSERT(!str[currentIndex], "Entire string not searched: %s", str);
	return false;
}
