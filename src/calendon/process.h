#ifndef CN_PROCESS_H
#define CN_PROCESS_H

#include <calendon/cn.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32

// https://docs.microsoft.com/en-us/windows/win32/psapi/enumerating-all-modules-for-a-process?redirectedfrom=MSDN
/**
 * Prints all the loaded DLLs to the log.
 */
CN_API bool cnProc_PrintLoadedDLLs(void);

#endif /* _WIN32 */

#ifdef __cplusplus
}
#endif

#endif /* CN_PROCESS_H */
