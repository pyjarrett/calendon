#ifndef KN_ASSETS_H
#define KN_ASSETS_H

#include <knell/kn.h>
#include <knell/path.h>

KN_API bool Assets_IsReady(void);
KN_API void Assets_Init(const char* assetDir);
KN_API void Assets_Shutdown(void);
KN_API_DEPRECATED("alpha", "Use PathBuffer functions instead", KN_API bool Assets_PathFor(const char* assetName, char* buffer, uint32_t bufferSize));
KN_API bool Assets_PathBufferFor(const char* assetName, PathBuffer* path);

#endif /* KN_ASSETS_H */
