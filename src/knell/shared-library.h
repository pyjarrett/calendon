#ifndef KNELL_SHARED_LIBRARY_H
#define KNELL_SHARED_LIBRARY_H

/**
 * Shared libraries (or DLLs) provide loading of code at runtime. The mechanism
 * for doing this and referencing libraries varies by platform, so provide a
 * common interface.
 */
#ifdef _WIN32

#include <knell/compat-windows.h>
typedef HMODULE knSharedLibrary;

#endif /* _WIN32 */

#ifdef __linux__

#include <dlfcn.h>
typedef void* knSharedLibrary;

#endif /* __linux__ */

void SharedLibrary_Release(knSharedLibrary library);
knSharedLibrary SharedLibrary_Load(const char* sharedLibraryName);
void* SharedLibrary_LookupFn(knSharedLibrary library, const char* fnName);

#endif //KNELL_SHARED_LIBRARY_H
