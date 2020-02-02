#include "assets-fileio.h"

#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

extern LogHandle LogSysAssets;

/**
 * Reads file assets according to a specific file type format (binary or text).
 * Text appends a trailing '\0' so functions requiring null-terminated input
 * work correctly.
 *
 * @param filename some valid, null-terminated path
 * @param format `KN_FILE_TYPE_BINARY` or `KN_FILE_TYPE_TEXT`
 * @param buffer where the file data will be written to
 * @return true if succeeded, false otherwise
 *
 * @see KN_FILE_TYPE_BINARY, KN_FILE_TYPE_TEXT
 */
bool Assets_ReadFile(const char *filename, uint32_t format, DynamicBuffer *buffer)
{
	if (!filename) {
		KN_ERROR(LogSysAssets, "Cannot read a null filename");
		return false;
	}

	if (!buffer) {
		KN_ERROR(LogSysAssets, "Cannot write file contents to a null buffer");
		return false;
	}

	if (format != KN_FILE_TYPE_BINARY && format != KN_FILE_TYPE_TEXT) {
		KN_ERROR(LogSysAssets, "Invalid file type constant. "
			"Must be KN_FILE_TYPE_BINARY or KN_FILE_TYPE_TEXT");
		return false;
	}

	const char* readMode = format == KN_FILE_TYPE_TEXT ? "r" : "rb";
	FILE* file = fopen(filename, readMode);
	if (!file) {
		KN_ERROR(LogSysAssets, "Cannot open file: %s", filename);
		return false;
	}

	fseek(file, 0, SEEK_END);
	long int fileLength = ftell(file);
	if (fileLength > UINT32_MAX) {
		KN_ERROR(LogSysMain, "File '%s' is too large to load into dynamic buffer: %li KiB",
			filename, fileLength / 1024L);
		return false;
	}
	if (format == KN_FILE_TYPE_TEXT) {
		fileLength += 1;
	}
	Mem_Allocate(buffer, (uint32_t)fileLength);

	fseek(file, 0, SEEK_SET);
	size_t amountRead = fread(buffer->contents, 1, (size_t)fileLength, file);
	KN_TRACE(LogSysAssets, "Read %zu bytes from %s", amountRead, filename);
	if (amountRead != (size_t)fileLength) {
		if (feof(file)) {
			KN_TRACE(LogSysAssets, "File EOF reached");
		} else if (ferror(file)) {
			KN_ERROR(LogSysAssets, "Error reading file: %s", filename);
		}
	}
	fclose(file);

	if (format == KN_FILE_TYPE_TEXT) {
		buffer->contents[fileLength - 1] = '\0';
	}

	buffer->size = (uint32_t)amountRead;

	return true;
}

bool Assets_LastModifiedTime(const char* filename, uint64_t* lastModifiedTime)
{
	if (!filename) {
		return false;
	}

	struct stat info;
	if (stat(filename, &info) != 0) {
		return false;
	}

	*lastModifiedTime = (uint64_t)info.st_mtime;
	return true;
}
