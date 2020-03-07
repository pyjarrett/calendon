#include "memory.h"

#include <knell/kn.h>

#include <knell/log.h>

static uint32_t MemOutstandingAllocations;
static uint32_t LogSysMemory;

KN_API void Mem_Init(void)
{
	MemOutstandingAllocations = 0;
	Log_RegisterSystem(&LogSysMemory, "Memory", KN_LOG_TRACE);
}

KN_API void Mem_Allocate(DynamicBuffer* buffer, uint32_t size)
{
	if (!buffer) {
		KN_ERROR(LogSysMemory, "Cannot allocate for a null DynamicBuffer");
		return;
	}

	if (size == 0) {
		KN_ERROR(LogSysMemory, "Refusing to allocate nothing for a DynamicBuffer.");
		return;
	}

	buffer->contents = malloc(size);
	if (!buffer->contents) {
		KN_ERROR(LogSysMemory, "Unable to allocate %" PRIu32 " bytes for DynamicBuffer", size);
	}
	buffer->size = size;
	++MemOutstandingAllocations;
}

KN_API void Mem_Free(DynamicBuffer* buffer)
{
	if (!buffer) {
		KN_ERROR(LogSysMemory, "Cannot release a null DynamicBuffer.");
		return;
	}
	KN_ASSERT(buffer->contents != 0, "DynamicBuffer has already been freed");
	free(buffer->contents);
	buffer->contents = NULL;

	if (MemOutstandingAllocations == 0) {
		KN_ERROR(LogSysMemory, "Double free of buffer %p", (void*)buffer);
	}
	else {
		--MemOutstandingAllocations;
	}
}

KN_API void Mem_Shutdown(void)
{
	if (MemOutstandingAllocations != 0) {
		//KN_ERROR(LogSysMemory, "Memory systems leaks: %" PRIu32, MemOutstandingAllocations);
	}
}