#include "fileio.h"

#include "log.h"

#include <stdio.h>
#include <stdlib.h>

bool File_Read(const char *filename, const uint32_t format, DynamicBuffer *buffer)
{
	if (!filename) {
		KN_ERROR(LogSysMain, "Cannot read a null filename");
		return false;
	}

	if (!buffer) {
		KN_ERROR(LogSysMain, "Cannot write file contents to a null buffer");
		return false;
	}

	if (format != KN_FILE_TYPE_BINARY && format != KN_FILE_TYPE_TEXT) {
		KN_ERROR(LogSysMain, "Invalid file type constant. "
			"Must be KN_FILE_TYPE_BINARY or KN_FILE_TYPE_TEXT");
		return false;
	}

	const char* readMode = format == KN_FILE_TYPE_TEXT ? "r" : "rb";
	FILE* file = fopen(filename, readMode);
	if (!file) {
		KN_ERROR(LogSysMain, "Cannot open file: %s", filename);
		return false;
	}

	fseek(file, 0, SEEK_END);
	long int fileLength = ftell(file);
	if (format == KN_FILE_TYPE_TEXT) {
		fileLength += 1;
	}
	Mem_Allocate(buffer, fileLength);

	fseek(file, 0, SEEK_SET);
	size_t amountRead = fread(buffer->contents, 1, fileLength, file);
	KN_TRACE(LogSysMain, "Read %zu bytes from %s", amountRead, filename);
	if (amountRead != fileLength) {
		if (feof(file)) {
			KN_TRACE(LogSysMain, "File EOF reached");
		} else if (ferror(file)) {
			KN_ERROR(LogSysMain, "Error reading file: %s", filename);
		}
	}
	fclose(file);

	if (format == KN_FILE_TYPE_TEXT) {
		buffer->contents[fileLength - 1] = '\0';
	}

	buffer->size = amountRead;

	return true;
}
