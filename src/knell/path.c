#include "path.h"

#include <string.h>

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
