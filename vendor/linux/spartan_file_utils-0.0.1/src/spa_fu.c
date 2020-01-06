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
	return S_ISDIR(info.st_mode);
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
	return !S_ISDIR(info.st_mode);
}

