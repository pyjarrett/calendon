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

KN_API bool Path_Append(const char* toAdd, char* current, uint32_t length)
{
	size_t currentLength = strlen(current);
	size_t finalLength = currentLength + sizeof(toAdd) + 1 + 1;
	if (finalLength >= length) {
		return false;
	}
	current[currentLength] = '/';
	strcpy(current + currentLength + 1, toAdd);
	return true;
}

KN_API bool PathBuffer_Join(PathBuffer* root, const char* suffix)
{
	KN_ASSERT(root != NULL, "Cannot append to a null root path");
	KN_ASSERT(suffix != NULL, "Trying to append a null path");
	return Path_Append(suffix, root->str, KN_PATH_MAX);
}
