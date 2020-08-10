#ifndef CN_ASSETS_CONFIG_H
#define CN_ASSETS_CONFIG_H

#include <calendon/cn.h>
#include <calendon/system.h>

typedef struct {
	CnPathBuffer assetDirPath;
} CnAssetsConfig;

CnCommandLineOptionList cnAssets_CommandLineOptionList(void);
void* cnAssets_Config(void);
void cnAssets_SetDefaultConfig(void* config);


#endif /* CN_ASSETS_CONFIG_H */
