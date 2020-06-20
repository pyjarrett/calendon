#ifndef CN_FILEIO_H
#define CN_FILEIO_H

#include <calendon/cn.h>

#include <calendon/memory.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	CnFileTypeBinary,
	CnFileTypeText
} CnFileType;

CN_API bool cnAssets_ReadFile(const char *filename, uint32_t format, CnDynamicBuffer *buffer);
CN_API bool cnAssets_LastModifiedTime(const char* filename, uint64_t* lastModifiedTime);

#ifdef __cplusplus
}
#endif

#endif /* CN_FILEIO_H */
