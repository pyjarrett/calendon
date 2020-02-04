#ifndef KN_PATH_H
#define KN_PATH_H

#include "kn.h"

/**
 * Maximum path length varies by system, but assume a reasonable case.
 */
#define KN_PATH_MAX 256

/*
 * Verify the path length assumption against the OS.
 */
#ifdef __linux__
#include <linux/limits.h>
	KN_STATIC_ASSERT(KN_PATH_MAX <= PATH_MAX,
		"Knell allows larger path sizes than the OS");
#endif

/**
 * Rather than proliferate a ridiculous number of buffers and sizes all over the
 * place, agree on a "reasonable" path length.  This has the side-benefit of
 * allowing for joins to occur without additional allocations, and allowing
 * for quick buffer creation on the stack.
 */
typedef struct {
	char path[KN_PATH_MAX];
	uint32_t length;
} PathBuffer;

KN_STATIC_ASSERT(KN_PATH_MAX <= sizeof(PathBuffer),
				 "PathBuffer is not big enough");

KN_API bool Path_Append(const char* toAdd, char* current, uint32_t length);

#endif /* KN_PATH_H */
