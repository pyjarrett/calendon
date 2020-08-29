#include "memory.h"

#include <calendon/cn.h>

#include <calendon/log.h>

static uint32_t MemOutstandingAllocations;
static uint32_t LogSysMemory;

bool cnMem_Init(void)
{
	MemOutstandingAllocations = 0;
	LogSysMemory = cnLog_RegisterSystem("Memory");
	return true;
}

void cnMem_Allocate(CnDynamicBuffer* buffer, uint32_t size)
{
	CN_ASSERT_PTR(buffer);

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
	CN_ASSERT_PTR(buffer);
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

CnPlugin cnMem_Plugin(void)
{
	return (CnPlugin) {
		.init = cnMem_Init,
		.shutdown = cnMem_Shutdown,
		.tick = NULL,
		.draw = NULL,
		.sharedLibrary = NULL
	};
}

CnSystem cnMem_System(void) {
	return (CnSystem) {
		.name = "Memory",
		.plugin = cnMem_Plugin,
		.options = cnSystem_NoOptions,
		.config = cnSystem_NoConfig,
		.setDefaultConfig = cnSystem_NoDefaultConfig
	};
}