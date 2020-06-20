#include "assets.h"

#include <calendon/cn.h>

#include <calendon/env.h>
#include <calendon/log.h>
#include <calendon/path.h>

#include <string.h>

#define MAX_ASSET_DIR_LENGTH 256
static char assetsRoot[MAX_ASSET_DIR_LENGTH + 1];
static uint32_t assetsRootLength = 0;
CnLogHandle LogSysAssets;
static bool assetsInitialized = false;

bool cnAssets_IsReady(void)
{
	return assetsInitialized;
}

/**
 * Initial the asset system with the top level directory where assets should
 * be found.
 */
void cnAssets_Init(const char* assetDir)
{
	if (cnAssets_IsReady()) {
		CN_FATAL_ERROR("Double initialization of assets system.");
	}
	if (strlen(assetDir) >= MAX_ASSET_DIR_LENGTH) {
		CN_FATAL_ERROR("Asset path root is too long.  Cannot initialize asset path with %s", assetDir);
	}
	strcpy(assetsRoot, assetDir);
	assetsRootLength = (uint32_t)strlen(assetsRoot);

	if (!cnPath_IsDir(assetsRoot)) {
		CnPathBuffer cwd;
		cnEnv_CurrentWorkingDirectory(cwd.str, CN_PATH_MAX);
		CN_FATAL_ERROR("Assets root directory doesn't exist: %s.  CWD: %s", assetsRoot, cwd.str);
	}

	cnLog_RegisterSystem(&LogSysAssets, "Assets", CnLogVerbosityTrace);

	CN_TRACE(LogSysAssets, "Assets initialized with root at: '%s'", assetsRoot);
	assetsInitialized = true;
}

void cnAssets_Shutdown(void)
{
	CN_ASSERT(cnAssets_IsReady(), "Cannot shutdown assets system, is not initialized.");
	assetsInitialized = false;
	CN_ASSERT(!cnAssets_IsReady(), "Shutdown did not work on assets system.");
}

bool cnAssets_PathBufferFor(const char* assetName, CnPathBuffer* path)
{
	if (assetsRootLength == 0) {
		CN_ERROR(LogSysAssets, "Asset system not initialized, cannot get path for %s", assetName);
		return false;
	}

	// Output buffer cannot hold root + '/' + assetName + '\0'.
	if (assetsRootLength + 1 + strlen(assetName) + 1 >= CN_PATH_MAX) {
		return false;
	}

	memcpy(&path->str, assetsRoot, assetsRootLength);
	path->str[assetsRootLength] = '/';
	strcpy(path->str + assetsRootLength + 1, assetName);

	CN_TRACE(LogSysAssets, "Resolved asset path '%s' -> '%s'", assetName, path->str);

	return true;
}
