
#include "env.h"

#ifdef _WIN32
	#include "compat-windows.h"
#elif __linux__
	#include <unistd.h>
#endif

KN_API bool Env_CurrentWorkingDirectory(char* buffer, uint32_t bufferSize)
{
	KN_ASSERT(buffer, "Must provide buffer to get current working directory");
#ifdef _WIN32
	//https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getcurrentdirectory
	return GetCurrentDirectory(bufferSize, buffer) != 0;
#elif __linux__
	return getcwd(buffer, bufferSize) != NULL;
#endif
}
