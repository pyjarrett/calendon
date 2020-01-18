#include "memory.h"

#include "kn.h"
#include "log.h"

static uint32_t MemOutstandingAllocations;
static uint32_t LogSysMemory;

void Mem_Init(void)
{
	MemOutstandingAllocations = 0;
	Log_RegisterSystem(&LogSysMemory, "Memory", KN_LOG_TRACE);
}

void Mem_Allocate(DynamicBuffer* buffer, const uint32_t size)
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
	++MemOutstandingAllocations;
}

void Mem_Free(DynamicBuffer* buffer)
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

void Mem_Shutdown(void)
{
	if (MemOutstandingAllocations != 0) {
		KN_ERROR(LogSysMemory, "Memory systems leaks: %" PRIu32, MemOutstandingAllocations);
	}
}