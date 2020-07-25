#include "string.h"

#include <calendon/cn.h>
#include <string.h>

bool cnString_TerminatedFitsIn(const char* str, const size_t bytes)
{
	CN_ASSERT_NOT_NULL(str);
	return strlen(str) + 1 < bytes;
}
