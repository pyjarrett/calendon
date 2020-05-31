#include "assets-fileio.h"

#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

extern CnLogHandle LogSysAssets;

/**
 * Reads file assets according to a specific file type format (binary or text).
 * Text appends a trailing '\0' so functions requiring null-terminated input
 * work correctly.
 *
 * @param filename some valid, null-terminated path
 * @param format `CN_FILE_TYPE_BINARY` or `CN_FILE_TYPE_TEXT`
 * @param buffer where the file data will be written to
 * @return true if succeeded, false otherwise
 *
 * @see CN_FILE_TYPE_BINARY, CN_FILE_TYPE_TEXT
 */
bool cnAssets_ReadFile(const char *filename, uint32_t format, CnDynamicBuffer *buffer)
{
	if (!filename) {
		CN_ERROR(LogSysAssets, "Cannot read a null filename");
		return false;
	}

	if (!buffer) {
		CN_ERROR(LogSysAssets, "Cannot write file contents to a null buffer");
		return false;
	}

	if (format != CN_FILE_TYPE_BINARY && format != CN_FILE_TYPE_TEXT) {
		CN_ERROR(LogSysAssets, "Invalid file type constant. "
			"Must be CN_FILE_TYPE_BINARY or CN_FILE_TYPE_TEXT");
		return false;
	}

	const char* readMode = format == CN_FILE_TYPE_TEXT ? "r" : "rb";
	FILE* file = fopen(filename, readMode);
	if (!file) {
		CN_ERROR(LogSysAssets, "Cannot open file: %s", filename);
		return false;
	}

	fseek(file, 0, SEEK_END);
	long int fileLength = ftell(file);
	if (fileLength > UINT32_MAX) {
		CN_ERROR(LogSysMain, "File '%s' is too large to load into dynamic buffer: %li KiB",
			filename, fileLength / 1024L);
		return false;
	}
	if (format == CN_FILE_TYPE_TEXT) {
		fileLength += 1;
	}
	cnMem_Allocate(buffer, (uint32_t) fileLength);

	fseek(file, 0, SEEK_SET);
	size_t amountRead = fread(buffer->contents, 1, (size_t)fileLength, file);
	CN_TRACE(LogSysAssets, "Read %zu bytes from %s", amountRead, filename);
	if (amountRead != (size_t)fileLength) {
		if (feof(file)) {
			CN_TRACE(LogSysAssets, "File EOF reached");
		} else if (ferror(file)) {
			CN_ERROR(LogSysAssets, "Error reading file: %s", filename);
		}
	}
	fclose(file);

	if (format == CN_FILE_TYPE_TEXT) {
		buffer->contents[fileLength - 1] = '\0';
	}

	buffer->size = (uint32_t)amountRead;

	return true;
}

bool cnAssets_LastModifiedTime(const char* filename, uint64_t* lastModifiedTime)
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
