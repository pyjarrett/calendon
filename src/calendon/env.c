#include "env.h"

#ifdef _WIN32
	#include <calendon/compat-windows.h>
#elif __linux__
	#include <unistd.h>
#endif

bool cnEnv_CurrentWorkingDirectory(char* buffer, uint32_t bufferSize)
{
	CN_ASSERT(buffer, "Must provide buffer to get current working directory");
#ifdef _WIN32
	//https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getcurrentdirectory
	return GetCurrentDirectory(bufferSize, buffer) != 0;
#elif __linux__
	return getcwd(buffer, bufferSize) != NULL;
#endif
}

bool cnEnv_DefaultCalendonHome(CnPathBuffer* path)
{
    CN_ASSERT(path, "Cannot write the default Calendon home to a null CnPathBuffer.");

    /*
     * CALENDON_HOME should the root directory of the Calendon project, which should
     * also be the root of the git repo.
     */
    const char* calendonHomeEnvVar = getenv("CALENDON_HOME");
    if (calendonHomeEnvVar) {
		cnPathBuffer_Set(path, calendonHomeEnvVar);
        return true;
    }

    return cnEnv_CurrentWorkingDirectory(path->str, CN_MAX_TERMINATED_PATH);
}