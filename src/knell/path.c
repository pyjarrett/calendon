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
