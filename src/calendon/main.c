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
	if (cnPathBuffer_IsFile(&config->gameLibPath)) {
		cnMain_LoadPayload(config);
	}
	else {
		CN_FATAL_ERROR("Only demos are currently supported.");
	}

	// Initialize the time of the first program tick, so tick deltas are
	// relevant after this point.
	s_lastTick = cnTime_MakeNow();

	CN_TRACE(LogSysMain, "Systems initialized.");
}

void cnMain_AllBeginFrame(CnFrameEvent* event)
{
	CN_ASSERT_PTR(event);
	for (uint32_t i = 0; i < s_numCoreSystems; ++i) {
		cnBehavior_BeginFrame(&s_coreSystems[i].behavior, event);
	}
}

void cnMain_AllTick(CnFrameEvent* event)
{
	CN_ASSERT_PTR(event);
	for (uint32_t i = 0; i < s_numCoreSystems; ++i) {
		cnBehavior_Tick(&s_coreSystems[i].behavior, event);
	}
}

void cnMain_AllDraw(CnFrameEvent* event)
{
	CN_ASSERT_PTR(event);
	for (uint32_t i = 0; i < s_numCoreSystems; ++i) {
		cnBehavior_Draw(&s_coreSystems[i].behavior, event);
	}
}

void cnMain_AllEndFrame(CnFrameEvent* event)
{
	CN_ASSERT_PTR(event);
	for (uint32_t i = 0; i < s_numCoreSystems; ++i) {
		cnBehavior_EndFrame(&s_coreSystems[i].behavior, event);
	}
}

/**
 * The big loop which processes events, ticks and draws until the game is ready
 * to shut down.
 */
void cnMain_Loop(void)
{
	CnFrameEvent event;
	event.dt = cnTime_MakeZero();

	while (cnMain_IsRunning() && !cnMain_IsTickLimitReached())
	{
		cnMain_AllBeginFrame(&event);

		cnUI_ProcessWindowEvents();

		// Event checking should be quick.  Always processing events prevents
		// slowness due to bursts.
		cnUI_ProcessWindowEvents();

		if (cnMain_GenerateTick(&event.dt)) {
			cnMain_AllBeginFrame(&event);
			cnMain_AllTick(&event);

			cnMain_TickCompleted();

			cnMain_AllDraw(&event);
			cnMain_AllEndFrame(&event);
		}

		// cnUI_EndFrame();
	}
}

void cnMain_Shutdown(void)
{
	cnR_Shutdown();
	cnUI_Shutdown();

	// Shutdown all systems in reverse order.
	for (uint32_t i = 0; i < s_numCoreSystems; ++i) {
		const uint32_t nextSystemIndex = s_numCoreSystems - i - 1;

		CnSystem* system = &s_coreSystems[nextSystemIndex];
		if (!system->shutdown) {
			cnPrint("No shutdown function for: %s\n", system->name());
		}
		else {
			system->shutdown();
		}
	}
}
