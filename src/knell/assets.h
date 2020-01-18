#ifndef KN_ASSETS_H
#define KN_ASSETS_H

#include "kn.h"

KN_API void Assets_Init(const char* assetDir);
KN_API bool Assets_PathFor(const char* assertName, char* buffer, uint32_t bufferSize);

#endif /* KN_ASSETS_H */
