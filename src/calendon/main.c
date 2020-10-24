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
	// Builds the list of the systems known from program initialization to load
	// and use.
	cnMain_BuildCoreSystemList();

	// Log filters exist as part of the command line, so pre-initialization of
	// the log is required to be able to apply the filters.
	cnLog_PreInit();

	// Command line parsing requires the availability of systems for parsing of
	// arguments.  Systems which allow command-line configuration must then
	// have already been determined and described.
	//
	// Command line parsing allow systems to provide overrides for their
	// default configurations, or for their configurations as loaded from file.
	if (!cnMain_ParseCommandLine(argc, argv)) {
		CN_FATAL_ERROR("Unable to parse command line.");
	}

	// Configuration of systems is complete at this point, so initialize systems.
	cnMain_InitCoreSystems();

	// Calendon could be used for headless programs, such as a server for
	// multiplayer play.
	CnMainConfig* config = (CnMainConfig*) cnMain_Config();
	if (!config->headless) {
		cnMain_StartUpUI();
	}

	// If there is a demo to load from file, then use that.
	cnMain_LoadPayload(config);

	// Initialize the time of the first program tick, so tick deltas are
	// relevant after this point.
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
		// cnUI_StartFrame();

		// Event checking should be quick.  Always processing events prevents
		// slowness due to bursts.
		cnUI_ProcessWindowEvents();

		CnTime dt;
		if (cnMain_GenerateTick(&dt)) {
			s_payload.tick(dt);
			cnMain_TickCompleted();
			s_payload.draw();
		}

		// cnPayload_Draw(s_payload);

		// cnUI_EndFrame();
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
