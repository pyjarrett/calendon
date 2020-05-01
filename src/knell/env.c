#include "env.h"

#ifdef _WIN32
	#include <knell/compat-windows.h>
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

KN_API bool Env_DefaultKnellHome(PathBuffer* buffer)
{
    KN_ASSERT(buffer, "Cannot write the default Knell home to a null PathBuffer.");

    /*
     * KNELL_HOME should the root directory of the Knell project, which should
     * also be the root of the git repo.
     */
    const char* knellHomeEnvVar = getenv("KNELL_HOME");
    if (knellHomeEnvVar) {
        PathBuffer_Set(buffer, knellHomeEnvVar);
        return true;
    }

    return Env_CurrentWorkingDirectory(buffer->str, KN_PATH_MAX);
}