#include "path.h"

#include <calendon/env.h>

#include <string.h>
#include <sys/stat.h>

bool cnPath_Exists(const char* path)
{
	if (!path) {
		return false;
	}
	struct stat info;
	return stat(path, &info) == 0;
}

bool cnPath_IsDir(const char* path)
{
	if (!path) {
		return false;
	}

	struct stat info;
	if (stat(path, &info) != 0) {
		return false;
	}

#ifdef _WIN32
	return (info.st_mode & S_IFDIR) == S_IFDIR;
#else
	return S_ISDIR(info.st_mode);
#endif
}

bool cnPath_IsFile(const char* path)
{
	if (!path) {
		return false;
	}

	struct stat info;
	if (stat(path, &info) != 0) {
		return false;
	}
#ifdef _WIN32
	return (info.st_mode & S_IFREG) == S_IFREG;
#else
	return !S_ISDIR(info.st_mode);
#endif
}

bool cnPath_Append(const char* toAdd, char* current, uint32_t length)
{
	const size_t currentLength = strlen(current);
	const size_t finalLength = currentLength + strlen(toAdd) + 1 + 1;
	if (finalLength >= length) {
		return false;
	}
	current[currentLength] = '/';
	strcpy(current + currentLength + 1, toAdd);
	return true;
}

void cnPathBuffer_Clear(CnPathBuffer* path)
{
	CN_ASSERT(path != NULL, "Cannot clear a null CnPathBuffer.");
	memset(path->str, 0, CN_PATH_MAX + 1);
}

bool cnPathBuffer_Set(CnPathBuffer* path, const char* initialPath)
{
	CN_ASSERT(path != NULL, "Cannot assign to a null CnPathBuffer");
	CN_ASSERT(initialPath != NULL, "Cannot assign a null initial path to a CnPathBuffer");
	const size_t initialPathLength = strlen(initialPath);
	if (initialPathLength < CN_PATH_MAX + 1) {
		strcpy(path->str, initialPath);
		return true;
	}
	return false;
}

bool cnPathBuffer_Join(CnPathBuffer* root, const char* suffix)
{
	CN_ASSERT(root != NULL, "Cannot append to a null root path");
	CN_ASSERT(suffix != NULL, "Trying to append a null path");
	return cnPath_Append(suffix, root->str, CN_PATH_MAX);
}

bool cnPathBuffer_Exists(CnPathBuffer* path)
{
	CN_ASSERT(path != NULL, "Cannot check path existence for a null CnPathBuffer.");
	return cnPath_Exists(path->str);
}

bool cnPathBuffer_IsDir(CnPathBuffer* path)
{
	CN_ASSERT(path != NULL, "Cannot check to see if a null CnPathBuffer is a directory.");
	return cnPath_IsDir(path->str);
}

bool cnPathBuffer_IsFile(CnPathBuffer* path)
{
	CN_ASSERT(path != NULL, "Cannot check to see if a null CnPathBuffer is a file.");
	return cnPath_IsFile(path->str);
}

bool cnPathBuffer_CurrentWorkingDirectory(CnPathBuffer* path)
{
	CN_ASSERT(path != NULL, "Cannot put a current working directory into a null CnPathBuffer.");
	return cnEnv_CurrentWorkingDirectory(path->str, CN_PATH_MAX);
}