#ifndef CN_STRING_H
#define CN_STRING_H

/**
 * @file string.h
 *
 * Additional string helpers for Calendon.
 */

#include <calendon/cn.h>

#ifdef __cplusplus
extern "C" {
#endif

CN_API bool cnString_TerminatedFitsIn(const char* str, const size_t bytes);

#ifdef __cplusplus
}
#endif

#endif /* CN_STRING_H */
