#ifndef CN_ASSETS_H
#define CN_ASSETS_H

#include <calendon/cn.h>
#include <calendon/path.h>

#ifdef __cplusplus
extern "C" {
#endif

CN_API bool cnAssets_IsReady(void);
CN_API void cnAssets_Init(const char* assetDir);
CN_API void cnAssets_Shutdown(void);
CN_API_DEPRECATED("alpha", "Use CnPathBuffer functions instead", CN_API bool cnAssets_PathFor(char* assetName, char* buffer, uint32_t bufferSize));
CN_API bool cnAssets_PathBufferFor(const char* assetName, CnPathBuffer* path);

#ifdef __cplusplus
}
#endif

#endif /* CN_ASSETS_H */
