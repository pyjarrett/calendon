#include "spa_fu/spa_fu.h"

#include <sys/stat.h>

bool SPA_PathExists(const char* path)
{
	if (!path) {
		return false;
	}
	struct stat info;
	return stat(path, &info) == 0;
}

bool SPA_IsDir(const char* path)
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

bool SPA_IsFile(const char* path)
{
	if (!path) {
		return false;
	}

	struct stat info;
	if (stat(path, &info) != 0) {
		return false;
	}
#ifdef _WIN32
	return (info.st_mode & S_IFREG) != S_IFREG;
#else
	return !S_ISDIR(info.st_mode);
#endif
}

