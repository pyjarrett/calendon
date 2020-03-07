/*
 * User interface integration for dispatching graphic calls and receiving
 * inputs.
 */
#ifndef KN_UI_H
#define KN_UI_H

#include <knell/kn.h>
#include <knell/input.h>

KN_API void UI_Init(uint32_t width, uint32_t height);
KN_API void UI_Shutdown(void);

/**
 * Call this to give the windowing system the opportunity to run its message
 * pump.  This function should be callable regardless of whether or not an
 * actual game tick occurred or will occur.
 *
 * In Win32 this might pump messages, on other systems it might poll inputs for
 * them to be distributed to other systems.
 */
KN_API void UI_ProcessWindowEvents(void);

typedef struct {
	KeyInputs keySet;
	Mouse mouse;
} Input;

/**
 * Reads data out of the input system.
 */
KN_API Input* UI_InputPoll(void);

#endif /* KN_UI_H */

