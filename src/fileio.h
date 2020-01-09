#ifndef KN_FILEIO_H
#define KN_FILEIO_H

#include <stdbool.h>
#include <stdint.h>

#include "memory.h"

enum {
	KN_FILE_TYPE_BINARY,
	KN_FILE_TYPE_TEXT
};

bool File_Read(const char* filename, DynamicBuffer* buffer, uint32_t fileType);

#endif /* KN_FILEIO_H */
