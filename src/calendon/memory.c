#include "memory.h"

#include <calendon/cn.h>

#include <calendon/log.h>

static uint32_t MemOutstandingAllocations;
static uint32_t LogSysMemory;

void cnMem_Init(void)
{
	MemOutstandingAllocations = 0;
	cnLog_RegisterSystem(&LogSysMemory, "Memory", CnLogVerbosityTrace);
}

void cnMem_Allocate(CnDynamicBuffer* buffer, uint32_t size)
{
	if (!buffer) {
		CN_ERROR(LogSysMemory, "Cannot allocate for a null CnDynamicBuffer");
		return;
	}

	if (size == 0) {
		CN_ERROR(LogSysMemory, "Refusing to allocate nothing for a CnDynamicBuffer.");
		return;
	}

	buffer->contents = malloc(size);
	if (!buffer->contents) {
		CN_ERROR(LogSysMemory, "Unable to allocate %" PRIu32 " bytes for CnDynamicBuffer", size);
	}
	buffer->size = size;
	++MemOutstandingAllocations;
}

void cnMem_Free(CnDynamicBuffer* buffer)
{
	if (!buffer) {
		CN_ERROR(LogSysMemory, "Cannot release a null CnDynamicBuffer.");
		return;
	}
	CN_ASSERT(buffer->contents != 0, "CnDynamicBuffer has already been freed");
	free(buffer->contents);
	buffer->contents = NULL;

	if (MemOutstandingAllocations == 0) {
		CN_ERROR(LogSysMemory, "Double free of buffer %p", (void*)buffer);
	}
	else {
		--MemOutstandingAllocations;
	}
}

void cnMem_Shutdown(void)
{
	if (MemOutstandingAllocations != 0) {
		//CN_ERROR(LogSysMemory, "Memory systems leaks: %" PRIu32, MemOutstandingAllocations);
	}
}