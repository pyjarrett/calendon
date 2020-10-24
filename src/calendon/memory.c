#include "memory.h"

#include <calendon/cn.h>

#include <calendon/log.h>

static CnLogHandle LogSysMemory;
static uint32_t s_outstandingDynamicBuffers;

bool cnMemory_Init(void)
{
	s_outstandingDynamicBuffers = 0;
	LogSysMemory = cnLog_RegisterSystem("Memory");
	return true;
}

void cnDynamicBuffer_Allocate(CnDynamicBuffer* buffer, uint32_t size)
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
	++s_outstandingDynamicBuffers;
}

void cnDynamicBuffer_Free(CnDynamicBuffer* buffer)
{
	CN_ASSERT_PTR(buffer);
	CN_ASSERT(buffer->contents != 0, "CnDynamicBuffer has already been freed");
	free(buffer->contents);
	buffer->contents = NULL;

	if (s_outstandingDynamicBuffers == 0) {
		CN_ERROR(LogSysMemory, "Double free of buffer %p", (void*)buffer);
	}
	else {
		--s_outstandingDynamicBuffers;
	}
}

void cnMemory_Shutdown(void)
{
	if (s_outstandingDynamicBuffers != 0) {
		//CN_ERROR(LogSysMemory, "Memory systems leaks: %" PRIu32, s_outstandingDynamicBuffers);
	}
}

CnBehavior cnMemory_Plugin(void)
{
	return (CnBehavior) {
		.init = cnMemory_Init,
		.shutdown = cnMemory_Shutdown,
		.tick = NULL,
		.draw = NULL,
		.sharedLibrary = NULL
	};
}

CnSystem cnMemory_System(void) {
	return (CnSystem) {
		.name = "Memory",
		.plugin = cnMemory_Plugin,
		.options = cnSystem_NoOptions,
		.config = cnSystem_NoConfig,
		.setDefaultConfig = cnSystem_NoDefaultConfig
	};
}