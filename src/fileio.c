#include "fileio.h"

#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

static void* File_Allocate(uint32_t size)
{
	return malloc(size);
}

static void File_Free(void* ptr)
{
	free(ptr);
}

bool File_Read(const char* filename, MemoryFile* memoryFile, uint32_t format) {
	if (!filename) {
		KN_ERROR(LogSysMain, "Cannot read a null filename");
		return false;
	}

	if (!memoryFile) {
		KN_ERROR(LogSysMain, "Cannot write to a null FileContents");
		return false;
	}

	if (format != KN_FILE_TYPE_BINARY && format != KN_FILE_TYPE_TEXT) {
		KN_ERROR(LogSysMain, "Invalid file type constant. "
			"Must be KN_FILE_TYPE_BINARY or KN_FILE_TYPE_TEXT")
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

	// Allocate space for file contents.
	char* contents = (char*)File_Allocate(fileLength);
	if (!contents) {
		KN_ERROR(LogSysMain, "Unable to allocate memory to read file: %s", filename);
		return false;
	}

	fseek(file, 0, SEEK_SET);
	size_t amountRead = fread(contents, 1, fileLength, file);
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
		contents[fileLength - 1] = '\0';
	}

	memoryFile->contents = contents;
	memoryFile->size = amountRead;

	return true;
}

void File_Release(MemoryFile* file)
{
	if (!file) {
		KN_ERROR(LogSysMain, "Cannot release a null memory file.");
	}

	File_Free(file->contents);
}
