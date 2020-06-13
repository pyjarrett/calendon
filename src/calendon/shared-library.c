#include "shared-library.h"

#include <calendon/cn.h>

#ifdef _WIN32

void cnSharedLibrary_Release(CnSharedLibrary library)
{
	if (library) {
		FreeLibrary(library);
	}
}

CnSharedLibrary cnSharedLibrary_Load(const char* sharedLibraryName)
{
	CN_ASSERT(sharedLibraryName, "Cannot load a shared library with no name.");
	return LoadLibrary(sharedLibraryName);
}

void* cnSharedLibrary_LookupFn(CnSharedLibrary library, const char* fnName)
{
	CN_ASSERT(library, "Cannot load functions from a null shared library.");
	CN_ASSERT(fnName, "Cannot load a null function name from a shared library.");
    return GetProcAddress(library, fnName);
}

#endif /* _WIN32 */

#ifdef __linux__

void cnSharedLibrary_Release(CnSharedLibrary library)
{
	if (library) {
		dlclose(library);
	}
}

CnSharedLibrary cnSharedLibrary_Load(const char* sharedLibraryName)
{
	CN_ASSERT(sharedLibraryName, "Cannot load a shared library with no name.");
	return dlopen(sharedLibraryName,  RTLD_NOW);
}

void* cnSharedLibrary_LookupFn(CnSharedLibrary library, const char* fnName)
{
	CN_ASSERT(library, "Cannot load functions from a null shared library.");
	CN_ASSERT(fnName, "Cannot load a null function name from a shared library.");
	return dlsym(library, fnName);
}

#endif /* __linux__ */
