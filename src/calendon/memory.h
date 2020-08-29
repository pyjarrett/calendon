#ifndef CN_MEMORY_H
#define CN_MEMORY_H

#include <calendon/cn.h>

#include <calendon/system.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A single contiguous block of dynamically allocated memory.
 *
 * Allocated with `cnDynamicBuffer_Allocate` and then released with `cnDynamicBuffer_Free`.  The number
 * of unfree'd buffers is reported on shutdown.
 */
typedef struct {
	char* contents;
	uint32_t size;
} CnDynamicBuffer;

CN_API void cnDynamicBuffer_Allocate(CnDynamicBuffer* buffer, uint32_t size);
CN_API void cnDynamicBuffer_Free(CnDynamicBuffer* buffer);

CnSystem cnMemory_System(void);

#ifdef __cplusplus
}
#endif

#endif /* CN_MEMORY_H */
