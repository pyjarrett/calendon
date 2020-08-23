#ifndef CN_STRING_H
#define CN_STRING_H

/**
 * @file string.h
 *
 * Additional string helpers for Calendon.
 *
 * C strings, lists of characters with the end terminated by a null character
 * ('\0') cause all sorts of issues.  These provided functions use explicit
 * naming to reduce usage errors.
 *
 * These functions are not for strings dealing with content needing localization.
 */

#include <calendon/cn.h>

#ifdef __cplusplus
extern "C" {
#endif

CN_API bool cnString_FitsWithNull(const char* str, const size_t bytes);

CN_API bool cnString_NumCharacterBytes(const char* str, const size_t maxCharBytes, size_t* outLength);
CN_API bool cnString_StorageRequired(const char* str, const size_t maxBytes, size_t* outLength);

CN_API bool cnString_Copy(char* destination, const char* source, const size_t maxCharBytes);
CN_API bool cnString_Equal(const char* left, const char* right, const size_t maxBytes);
CN_API bool cnString_HasPrefix(const char* str, const size_t maxCharBytes, const char* prefix);

CN_API bool cnString_FirstIndexOfChar(const char* str, const size_t maxBytes, char ch, size_t* index);

#ifdef __cplusplus
}
#endif

#endif /* CN_STRING_H */
