#ifndef CN_ASSETS_H
#define CN_ASSETS_H

#include <calendon/cn.h>
#include <calendon/path.h>
#include <calendon/system.h>

#ifdef __cplusplus
extern "C" {
#endif

CN_API CnSystem cnAssets_System(void);
CN_API bool cnAssets_IsReady(void);
CN_API bool cnAssets_PathBufferFor(const char* assetName, CnPathBuffer* path);

#ifdef __cplusplus
}
#endif

#endif /* CN_ASSETS_H */
