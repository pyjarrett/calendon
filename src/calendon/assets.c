#include "assets.h"

#include <calendon/cn.h>

#include <calendon/assets-config.h>
#include <calendon/log.h>
#include <calendon/path.h>
#include <calendon/string.h>

#include <string.h>

static char assetsRoot[CN_MAX_TERMINATED_PATH];
static size_t assetsRootLength = 0;
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
bool cnAssets_Init(void)
{
	CnAssetsConfig* config = (CnAssetsConfig*)cnAssets_Config();
	if (cnAssets_IsReady()) {
		CN_FATAL_ERROR("Double initialization of assets system.");
		return false;
	}
	if (!cnString_FitsWithNull(config->assetDirPath.str, CN_MAX_TERMINATED_PATH)) {
		CN_FATAL_ERROR("Asset path root is too long.  Cannot initialize asset path with %s", &config->assetDirPath.str);
		return false;
	}
	strcpy(assetsRoot, config->assetDirPath.str);
	assetsRootLength = strlen(assetsRoot);

	if (!cnPath_IsDir(assetsRoot)) {
		CnPathBuffer cwd;
		if (!cnPathBuffer_CurrentWorkingDirectory(&cwd)) {
			CN_FATAL_ERROR("Unable to get current working directory.");
		}
		CN_FATAL_ERROR("Assets root directory doesn't exist: %s.  CWD: %s", assetsRoot, cwd.str);
		return false;
	}

	LogSysAssets = cnLog_RegisterSystem("Assets");

	CN_TRACE(LogSysAssets, "Assets initialized with root at: '%s'", assetsRoot);
	assetsInitialized = true;
	return true;
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
	if (assetsRootLength + 1 + strlen(assetName) + 1 >= CN_MAX_TERMINATED_PATH) {
		return false;
	}

	memcpy(&path->str, assetsRoot, assetsRootLength);
	path->str[assetsRootLength] = '/';
	strcpy(path->str + assetsRootLength + 1, assetName);

	CN_TRACE(LogSysAssets, "Resolved asset path '%s' -> '%s'", assetName, path->str);

	return true;
}

CnBehavior cnAssets_Plugin(void) {
	return (CnBehavior) {
		.init          = cnAssets_Init,
		.shutdown      = cnAssets_Shutdown,
		.tick          = NULL,
		.draw          = NULL,
		.sharedLibrary = NULL
	};
}

CnSystem cnAssets_System(void)
{
	return (CnSystem) {
		.name             = "Assets",
		.options          = cnAssets_CommandLineOptionList,
		.config           = cnAssets_Config,
		.setDefaultConfig = cnAssets_SetDefaultConfig,
		.plugin           = cnAssets_Plugin
	};
}
