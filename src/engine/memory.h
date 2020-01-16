#ifndef KN_MEMORY_H
#define KN_MEMORY_H

#include <stdint.h>

typedef struct {
	char* contents;
	uint32_t size;
	const char* file;
	uint64_t line;
} DynamicBuffer;

void Mem_Init();
void Mem_Shutdown();
void Mem_Allocate(DynamicBuffer* buffer, uint32_t size);
void Mem_Free(DynamicBuffer* buffer);

#endif /* KN_MEMORY_H */

void Mem_Shutdown();