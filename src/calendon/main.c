#include "main.h"

#include <calendon/control.h>
#include <calendon/log.h>
#include <calendon/main-config.h>
#include <calendon/main-detail.h>
#include <calendon/tick-limits.h>
#include <calendon/render.h>
#include <calendon/ui.h>

/**
 * The initial startup point for Calendon.
 */
void cnMain_StartUp(int argc, char** argv)
{
	cnMain_BuildCoreSystemList();
	cnLog_PreInit();

	if (!cnMain_ParseCommandLine(argc, argv)) {
		CN_FATAL_ERROR("Unable to parse command line.");
	}

	for (uint32_t i = 0; i < s_numCoreSystems; ++i) {
		if (!s_coreSystems[i].plugin().init()) {
			CN_FATAL_ERROR("Unable to initialize core system: %d", i);
		}
	}

	CnMainConfig* config = (CnMainConfig*) cnMain_Config();
	if (!config->headless) {
		cnMain_StartUpUI();
	}

	// If there is a demo to load from file, then use that.
	if (!cnPlugin_IsComplete(&config->payload)) {
		if (!cnPath_IsFile(config->gameLibPath.str)) {
			CN_FATAL_ERROR("Cannot load game. '%s' is not a game library.", config->gameLibPath.str);
		}

		const char* gameLib = config->gameLibPath.str;
		if (gameLib) {
			cnMain_LoadPayloadFromFile(gameLib);
		}
	}
	else {
		cnMain_ValidatePayload(&config->payload);
	}

	s_payload.init();
	s_lastTick = cnTime_MakeNow();

	CN_TRACE(LogSysMain, "Systems initialized.");
}

/**
 * The big loop which processes events, ticks and draws until the game is ready
 * to shut down.
 */
void cnMain_Loop(void)
{
	CN_ASSERT(s_payload.tick, "Tick function not defined.");
	CN_ASSERT(s_payload.draw, "Draw function not defined.");

	while (cnMain_IsRunning() && !cnMain_IsTickLimitReached())
	{
		// Event checking should be quick.  Always processing events prevents
		// slowness due to bursts.
		cnUI_ProcessWindowEvents();

		CnTime dt;
		if (cnMain_GenerateTick(&dt)) {
			s_payload.tick(dt);
			cnMain_TickCompleted();
		}
		s_payload.draw();
	}
}

void cnMain_Shutdown(void)
{
	if (s_payload.shutdown) s_payload.shutdown();

	cnR_Shutdown();
	cnUI_Shutdown();

	for (uint32_t i = 0; i < s_numCoreSystems; ++i) {
		const uint32_t nextSystemIndex = s_numCoreSystems - i - 1;

		CnSystem* system = &s_coreSystems[nextSystemIndex];
		if (!system->plugin().shutdown) {
			cnPrint("No shutdown function for: %s\n", system->name);
		}
		else {
			system->plugin().shutdown();
		}
	}
}
