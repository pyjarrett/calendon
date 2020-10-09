#ifndef CN_MAIN_DETAIL_H
#define CN_MAIN_DETAIL_H

#include <calendon/cn.h>

#include <calendon/log.h>
#include <calendon/plugin.h>
#include <calendon/system.h>
#include <calendon/time.h>

#ifdef __cplusplus
extern "C" {
#endif

extern CnTime s_lastTick;
extern CnPlugin s_payload;

enum { CnMaxNumCoreSystems = 16 };
extern CnSystem s_coreSystems[CnMaxNumCoreSystems];
extern uint32_t s_numCoreSystems;

CnPlugin cnMain_Plugin(void);
CnSystem cnMain_System(void);

void cnMain_BuildCoreSystemList(void);
bool cnMain_ParseCommandLine(int argc, char** argv);

void cnMain_StartUpUI(void);
void cnMain_LoadPayloadFromFile(const char* sharedLibraryName);
void cnMain_ValidatePayload(CnPlugin* payload);
bool cnMain_GenerateTick(CnTime* outDt);

#ifdef __cplusplus
}
#endif

#endif /* CN_MAIN_DETAIL_H */