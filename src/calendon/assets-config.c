#include "assets-config.h"

#include <calendon/cn.h>

#include <calendon/path.h>
#include <calendon/string.h>

int32_t cnAssets_OptionAssetDir(const CnCommandLineParse* parse, void* c);

static CnAssetsConfig s_config;
static CnCommandLineOption options[] = {
	{
		"\t-a,--asset-dir DIR\n"
			"\t\tChange the directory for assets.\n",
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

int32_t cnAssets_OptionAssetDir(const CnCommandLineParse* parse, void* c)
{
	CN_ASSERT_PTR(parse);
	CN_ASSERT_PTR(c);

	CnAssetsConfig* config = (CnAssetsConfig*)c;

	if (!cnCommandLineParse_HasLookAhead(parse, 2)) {
		cnPrint("Must provide an asset directory to use.\n");
		return CnOptionParseError;
	}

	const char* assetDir = cnCommandLineParse_LookAhead(parse, 2);
	if (cnString_FitsWithNull(assetDir, CN_MAX_TERMINATED_PATH)) {
		if (!cnPath_IsDir(assetDir)) {
			cnPrint("Asset directory %s does not exist\n", assetDir);
			return CnOptionParseError;
		}
		cnPathBuffer_Set(&config->assetDirPath, assetDir);
		cnPrint("Asset path: '%s'\n", config->assetDirPath.str);
		return 2;
	}
	else {
		cnPrint( "The asset path is too long.");
		return CnOptionParseError;
	}
}

void* cnAssets_Config(void) {
	return &s_config;
}

void cnAssets_SetDefaultConfig(void* config)
{
	CnAssetsConfig* c = (CnAssetsConfig*)config;

	cnPathBuffer_Clear(&c->assetDirPath);
	if (!cnPathBuffer_DefaultCalendonHome(&c->assetDirPath)) {
		CN_FATAL_ERROR("Unable to get default Calendon home.");
	}
	if (!cnPathBuffer_Join(&c->assetDirPath, "assets")) {
		CN_FATAL_ERROR("Unable to join to Calendon home expect asset dir path: %s", c->assetDirPath.str);
	}
}
