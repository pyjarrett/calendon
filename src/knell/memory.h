#ifndef KN_MEMORY_H
#define KN_MEMORY_H

#include <stdint.h>

/**
 * A single contiguous block of dynamically allocated memory.
 *
 * Allocated with `Mem_Allocate` and then released with `Mem_Free`.  The number
 * of unfree'd buffers is reported on shutdown.
 */
typedef struct {
	char* contents;
	uint32_t size;
} DynamicBuffer;

void Mem_Init();
void Mem_Shutdown();
void Mem_Allocate(DynamicBuffer* buffer, uint32_t size);
void Mem_Free(DynamicBuffer* buffer);

#endif /* KN_MEMORY_H */
