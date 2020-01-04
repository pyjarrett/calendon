/*
 * User interface integration for dispatching graphic calls and receiving
 * inputs.
 */
#ifndef KN_UI_H
#define KN_UI_H

void initWindow();

/**
 * Call this to give the windowing system the opportunity to run its message
 * pump.  This function should be callable regardless of whether or not an
 * actual game tick occurred or will occur.
 *
 * In Win32 this might pump messages, on other systems it might poll inputs for
 * them to be distributed to other systems.
 */
void processWindowEvents();

#endif /* KN_UI_H */

