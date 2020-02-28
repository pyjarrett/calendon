#ifndef KN_PATH_H
#define KN_PATH_H

#include "kn.h"

/**
 * Maximum path length varies by system, but assume a reasonable case.
 */
#define KN_PATH_MAX 254

/*
 * Verify the path length assumption against the OS.
 */
#ifdef __linux__
#include <linux/limits.h>
	KN_STATIC_ASSERT(KN_PATH_MAX <= PATH_MAX,
		"Knell allows larger path sizes than the OS");
#endif

#ifdef _WIN32
	#include <minwindef.h>
	KN_STATIC_ASSERT(KN_PATH_MAX <= MAX_PATH,
		"Knell allows larger path sizes than the OS");
#endif

/**
 * Rather than proliferate a ridiculous number of buffers and sizes all over the
 * place, agree on a "reasonable" path length.  This has the side-benefit of
 * allowing for joins to occur without additional allocations, and allowing
 * for quick buffer creation on the stack.
 */
typedef struct {
	// Adds 1 byte for null terminator to maximum path length.
	char str[KN_PATH_MAX + 1];
} PathBuffer;

KN_STATIC_ASSERT(KN_PATH_MAX <= sizeof(PathBuffer),
				 "PathBuffer is not big enough");

KN_API bool Path_Exists(const char* path);
KN_API bool Path_IsDir(const char* path);
KN_API bool Path_IsFile(const char* path);

KN_API_DEPRECATED("alpha", "Use PathBuffer_Join instead", KN_API bool Path_Append(const char* toAdd, char* current, uint32_t length));
KN_API bool PathBuffer_Create(PathBuffer* path, const char* initialPath);
KN_API bool PathBuffer_Join(PathBuffer* root, const char* suffix);

#endif /* KN_PATH_H */
