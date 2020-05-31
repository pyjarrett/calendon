#ifndef CN_PATH_H
#define CN_PATH_H

#include <calendon/cn.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Maximum path length varies by system, but assume a reasonable case.
 */
#define CN_PATH_MAX 254

/*
 * Verify the path length assumption against the OS.
 */
#ifdef __linux__
#include <linux/limits.h>
	CN_STATIC_ASSERT(CN_PATH_MAX <= PATH_MAX,
		"Calendon allows larger path sizes than the OS");
#endif

#ifdef _WIN32
	#include <minwindef.h>
	CN_STATIC_ASSERT(CN_PATH_MAX <= MAX_PATH,
		"Calendon allows larger path sizes than the OS");
#endif

/**
 * Rather than proliferate a ridiculous number of buffers and sizes all over the
 * place, agree on a "reasonable" path length.  This has the side-benefit of
 * allowing for joins to occur without additional allocations, and allowing
 * for quick buffer creation on the stack.
 */
typedef struct {
	// Adds 1 byte for null terminator to maximum path length.
	char str[CN_PATH_MAX + 1];
} CnPathBuffer;

CN_STATIC_ASSERT(CN_PATH_MAX <= sizeof(CnPathBuffer),
				 "CnPathBuffer is not big enough");

CN_API bool cnPath_Exists(const char* path);
CN_API bool cnPath_IsDir(const char* path);
CN_API bool cnPath_IsFile(const char* path);

CN_API void cnPathBuffer_Clear(CnPathBuffer* path);
CN_API bool cnPathBuffer_Set(CnPathBuffer* path, const char* initialPath);
CN_API bool cnPathBuffer_Join(CnPathBuffer* root, const char* suffix);
CN_API bool cnPathBuffer_Exists(CnPathBuffer* path);
CN_API bool cnPathBuffer_IsDir(CnPathBuffer* path);
CN_API bool cnPathBuffer_IsFile(CnPathBuffer* path);

#ifdef __cplusplus
}
#endif

#endif /* CN_PATH_H */
