#include "path.h"

#include <string.h>
#include <sys/stat.h>

bool Path_Exists(const char* path)
{
	if (!path) {
		return false;
	}
	struct stat info;
	return stat(path, &info) == 0;
}

bool Path_IsDir(const char* path)
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

bool Path_IsFile(const char* path)
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

bool Path_Append(const char* toAdd, char* current, uint32_t length)
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

void PathBuffer_Clear(PathBuffer* path)
{
	KN_ASSERT(path != NULL, "Cannot clear a null PathBuffer.");
	memset(path->str, 0, KN_PATH_MAX + 1);
}

bool PathBuffer_Set(PathBuffer* path, const char* initialPath)
{
	KN_ASSERT(path != NULL, "Cannot assign to a null PathBuffer");
	KN_ASSERT(initialPath != NULL, "Cannot assign a null initial path to a PathBuffer");
	const size_t initialPathLength = strlen(initialPath);
	if (initialPathLength < KN_PATH_MAX + 1) {
		strcpy(path->str, initialPath);
		return true;
	}
	return false;
}

bool PathBuffer_Join(PathBuffer* root, const char* suffix)
{
	KN_ASSERT(root != NULL, "Cannot append to a null root path");
	KN_ASSERT(suffix != NULL, "Trying to append a null path");
	return Path_Append(suffix, root->str, KN_PATH_MAX);
}

bool PathBuffer_Exists(PathBuffer* path)
{
	KN_ASSERT(path != NULL, "Cannot check path existence for a null PathBuffer.");
	return Path_Exists(path->str);
}

bool PathBuffer_IsDir(PathBuffer* path)
{
	KN_ASSERT(path != NULL, "Cannot check to see if a null PathBuffer is a directory.");
	return Path_IsDir(path->str);
}

bool PathBuffer_IsFile(PathBuffer* path)
{
	KN_ASSERT(path != NULL, "Cannot check to see if a null PathBuffer is a file.");
	return Path_IsFile(path->str);
}
