#ifndef CN_SHARED_LIBRARY_H
#define CN_SHARED_LIBRARY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Shared libraries (or DLLs) provide loading of code at runtime. The mechanism
 * for doing this and referencing libraries varies by platform, so provide a
 * common interface.
 */
#ifdef _WIN32

#include <calendon/compat-windows.h>
typedef HMODULE CnSharedLibrary;

#endif /* _WIN32 */

#ifdef __linux__

#include <dlfcn.h>
typedef void* CnSharedLibrary;

#endif /* __linux__ */

void cnSharedLibrary_Release(CnSharedLibrary library);
CnSharedLibrary cnSharedLibrary_Load(const char* sharedLibraryName);
void* cnSharedLibrary_LookupFn(CnSharedLibrary library, const char* fnName);

#ifdef __cplusplus
}
#endif

#endif /* CN_SHARED_LIBRARY_H */
