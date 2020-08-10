#include "assets-config.h"

#include <calendon/cn.h>

#include <calendon/command-line-parse.h>
#include <calendon/string.h>
#include <calendon/system.h>

static CnAssetsConfig s_config;

int32_t cnAssets_OptionAssetDir(const CnCommandLineParse* parse, void* c)
{
	CN_ASSERT_NOT_NULL(parse);
	CN_ASSERT_NOT_NULL(c);

	CnAssetsConfig* config = c;

	if (!cnCommandLineParse_HasLookAhead(parse, 2)) {
		printf("Must provide an asset directory to use.\n");
		return CnOptionParseError;
	}

	const char* assetDir = cnCommandLineParse_LookAhead(parse, 2);
	if (cnString_TerminatedFitsIn(assetDir, CN_MAX_TERMINATED_PATH)) {
		if (!cnPath_IsDir(assetDir)) {
			printf("Asset directory %s does not exist\n", assetDir);
			return CnOptionParseError;
		}
		cnPathBuffer_Set(&config->assetDirPath, assetDir);
		printf("Asset path: '%s'\n", config->assetDirPath.str);
		return 2;
	}
	else {
		printf( "The asset path is too long.");
		return CnOptionParseError;
	}
}

static CnCommandLineOption options[] = {
	{
		"-a,--asset-dir DIR         Change the directory for assets.\n",
		"-a",
		"--asset-dir",
		cnAssets_OptionAssetDir
	},
};

CnCommandLineOptionList cnAssets_CommandLineOptionList(void) {
	CnCommandLineOptionList optionList;
	optionList.options = options;
	optionList.numOptions = 1;
	return optionList;
}

void* cnAssets_Config(void) {
	return &s_config;
}

void cnAssets_SetDefaultConfig(void* config)
{
	CnAssetsConfig* c = (CnAssetsConfig*)config;
	cnPathBuffer_Clear(&c->assetDirPath);
}
