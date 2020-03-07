#include "assets.h"

#include <knell/kn.h>

#include <knell/env.h>
#include <knell/log.h>
#include <knell/path.h>

#include <string.h>

#define MAX_ASSET_DIR_LENGTH 256
static char assetsRoot[MAX_ASSET_DIR_LENGTH + 1];
static uint32_t assetsRootLength = 0;
LogHandle LogSysAssets;
static bool assetsInitialized = false;

KN_API bool Assets_IsReady(void)
{
	return assetsInitialized;
}

/**
 * Initial the asset system with the top level directory where assets should
 * be found.
 */
KN_API void Assets_Init(const char* assetDir)
{
	if (Assets_IsReady()) {
		KN_FATAL_ERROR("Double initialization of assets system.");
	}
	if (strlen(assetDir) >= MAX_ASSET_DIR_LENGTH) {
		KN_FATAL_ERROR("Asset path root is too long.  Cannot initialize asset path with %s", assetDir);
	}
	strcpy(assetsRoot, assetDir);
	assetsRootLength = (uint32_t)strlen(assetsRoot);

	if (!Path_IsDir(assetsRoot)) {
		PathBuffer cwd;
		Env_CurrentWorkingDirectory(cwd.str, KN_PATH_MAX);
		KN_FATAL_ERROR("Assets root directory doesn't exist: %s.  CWD: %s", assetsRoot, cwd.str);
	}

	Log_RegisterSystem(&LogSysAssets, "Assets", KN_LOG_TRACE);

	KN_TRACE(LogSysAssets, "Assets initialized with root at: '%s'", assetsRoot);
	assetsInitialized = true;
}

KN_API void Assets_Shutdown(void)
{
	KN_ASSERT(Assets_IsReady(), "Cannot shutdown assets system, is not initialized.");
	assetsInitialized = false;
	KN_ASSERT(!Assets_IsReady(), "Shutdown did not work on assets system.");
}

/**
 * Gets the path for accessing a specific resource within the assets system.
 */
KN_API bool Assets_PathFor(const char* assetName, char* buffer, uint32_t bufferSize)
{
	KN_WARN_DEPRECATED("Use PathBuffer* functions instead");
	if (assetsRootLength == 0) {
		KN_ERROR(LogSysAssets, "Asset system not initialized, cannot get path for %s", assetName);
		return false;
	}

	// Output buffer cannot hold root + '/' + assetName + '\0'.
	if (assetsRootLength + 1 + strlen(assetName) + 1 >= bufferSize) {
		return false;
	}

	memcpy(buffer, assetsRoot, assetsRootLength);
	buffer[assetsRootLength] = '/';
	strcpy(buffer + assetsRootLength + 1, assetName);

	KN_TRACE(LogSysAssets, "Resolved asset path '%s' -> '%s'", assetName, buffer);

	return true;
}

KN_API bool Assets_PathBufferFor(const char* assetName, PathBuffer* path)
{
	if (assetsRootLength == 0) {
		KN_ERROR(LogSysAssets, "Asset system not initialized, cannot get path for %s", assetName);
		return false;
	}

	// Output buffer cannot hold root + '/' + assetName + '\0'.
	if (assetsRootLength + 1 + strlen(assetName) + 1 >= KN_PATH_MAX) {
		return false;
	}

	memcpy(&path->str, assetsRoot, assetsRootLength);
	path->str[assetsRootLength] = '/';
	strcpy(path->str + assetsRootLength + 1, assetName);

	KN_TRACE(LogSysAssets, "Resolved asset path '%s' -> '%s'", assetName, path->str);

	return true;
}
