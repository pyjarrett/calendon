/*
 * Querying information about the program's environment.
 */
#ifndef KN_ENV_H
#define KN_ENV_H

#include <knell/kn.h>

/**
 * Places the current working directory into the buffer, returning false if the
 * buffer is not sufficienctly sized.
 */
KN_API bool Env_CurrentWorkingDirectory(char* buffer, uint32_t bufferSize);

#endif /* KN_ENV_H */
