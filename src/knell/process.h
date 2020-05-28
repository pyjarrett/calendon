#ifndef KN_PROCESS_H
#define KN_PROCESS_H

#include <knell/kn.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32

// https://docs.microsoft.com/en-us/windows/win32/psapi/enumerating-all-modules-for-a-process?redirectedfrom=MSDN
/**
 * Prints all the loaded DLLs to the log.
 */
KN_API bool Proc_PrintLoadedDLLs(void);

#endif /* _WIN32 */

#ifdef __cplusplus
}
#endif

#endif /* KN_PROCESS_H */
