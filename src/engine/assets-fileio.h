#ifndef KN_FILEIO_H
#define KN_FILEIO_H

#include <stdbool.h>
#include <stdint.h>

#include "memory.h"

enum {
	KN_FILE_TYPE_BINARY,
	KN_FILE_TYPE_TEXT
};

bool Assets_ReadFile(const char *filename, uint32_t format, DynamicBuffer *buffer);

#endif /* KN_FILEIO_H */
