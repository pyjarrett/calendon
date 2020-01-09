#ifndef KN_ASSETS_H
#define KN_ASSETS_H

#include <stdbool.h>
#include <stdint.h>

void assets_init(const char* assetDir);
bool assets_pathFor(const char* assertName, char* buffer, uint32_t bufferSize);

#endif /* KN_ASSETS_H */
