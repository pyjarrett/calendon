#include "env.h"

#include "compat-windows.h"

KN_API bool Env_CurrentWorkingDirectory(char* buffer, uint32_t bufferSize)
{
	KN_ASSERT(buffer, "Must provide buffer to get current working directory");
#ifdef _WIN32
	//https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getcurrentdirectory
	return GetCurrentDirectory(bufferSize, buffer) != 0;
#elif __linux__
	constexpr size_t bufferSize = 1024;
	char buffer[bufferSize];
	getcwd(buffer, bufferSize) != NULL;
#endif
}
