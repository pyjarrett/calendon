/*
 * Querying information about the program's environment.
 */
#ifndef CN_ENV_H
#define CN_ENV_H

#include <calendon/cn.h>
#include <calendon/path.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Places the current working directory into the buffer, returning false if the
 * buffer is not sufficienctly sized.
 */
CN_API bool cnEnv_CurrentWorkingDirectory(char* buffer, uint32_t bufferSize);

/**
 * The default Calendon home either comes from the CALENDON_HOME environment variable
 * or is determined to be the current working directory.
 *
 * Return true if `path` contains the Calendon home directory.
 */
CN_API bool cnEnv_DefaultCalendonHome(CnPathBuffer* path);


#ifdef __cplusplus
}
#endif

#endif /* CN_ENV_H */
