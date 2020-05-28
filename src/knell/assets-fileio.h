#ifndef KN_FILEIO_H
#define KN_FILEIO_H

#include <knell/kn.h>

#include <knell/memory.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	KN_FILE_TYPE_BINARY,
	KN_FILE_TYPE_TEXT
};

KN_API bool Assets_ReadFile(const char *filename, uint32_t format, DynamicBuffer *buffer);
KN_API bool Assets_LastModifiedTime(const char* filename, uint64_t* lastModifiedTime);

#ifdef __cplusplus
}
#endif

#endif /* KN_FILEIO_H */
