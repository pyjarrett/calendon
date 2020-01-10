#ifndef KN_ASSETS_H
#define KN_ASSETS_H

#include <stdbool.h>
#include <stdint.h>

void Assets_Init(const char* assetDir);
bool Assets_PathFor(const char* assertName, char* buffer, uint32_t bufferSize);

#endif /* KN_ASSETS_H */
