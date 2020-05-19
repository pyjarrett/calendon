#include "shared-library.h"

#include <knell/kn.h>

#ifdef _WIN32

void SharedLibrary_Release(knSharedLibrary library)
{
	if (library) {
		FreeLibrary(library);
	}
}

knSharedLibrary SharedLibrary_Load(const char* sharedLibraryName)
{
	KN_ASSERT(sharedLibraryName, "Cannot load a shared library with no name.");
	return LoadLibrary(sharedLibraryName);
}

void* SharedLibrary_LookupFn(knSharedLibrary library, const char* fnName)
{
	KN_ASSERT(library, "Cannot load functions from a null shared library.");
	KN_ASSERT(fnName, "Cannot load a null function name from a shared library.");
    return GetProcAddress(library, fnName);
}

#endif /* _WIN32 */

#ifdef __linux__

void SharedLibrary_Release(knSharedLibrary library)
{
	if (library) {
		dlclose(library);
	}
}

knSharedLibrary SharedLibrary_Load(const char* sharedLibraryName)
{
	KN_ASSERT(sharedLibraryName, "Cannot load a shared library with no name.");
	return dlopen(sharedLibraryName,  RTLD_NOW);
}

void* SharedLibrary_LookupFn(knSharedLibrary library, const char* fnName)
{
	KN_ASSERT(library, "Cannot load functions from a null shared library.");
	KN_ASSERT(fnName, "Cannot load a null function name from a shared library.");
	return dlsym(library, fnName);
}

#endif /* __linux__ */
