/*
 * Querying information about the program's environment.
 */
#ifndef KN_ENV_H
#define KN_ENV_H

#include <knell/kn.h>
#include <knell/path.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Places the current working directory into the buffer, returning false if the
 * buffer is not sufficienctly sized.
 */
KN_API bool Env_CurrentWorkingDirectory(char* buffer, uint32_t bufferSize);

/**
 * The default Knell home either comes from the KNELL_HOME environment variable
 * or is determined to be the current working directory.
 *
 * Return true if `path` contains the Knell home directory.
 */
KN_API bool Env_DefaultKnellHome(PathBuffer* path);


#ifdef __cplusplus
}
#endif

#endif /* KN_ENV_H */
