/*
 * User interface integration for dispatching graphic calls and receiving
 * inputs.
 */
#ifndef CN_UI_H
#define CN_UI_H

#include <calendon/cn.h>

#include <calendon/dimension.h>
#include <calendon/input-button-mapping.h>
#include <calendon/input-keyset.h>
#include <calendon/input-mouse.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	CnDimension2u32 resolution;
} CnUIInitParams;

CN_API void cnUI_Init(CnUIInitParams* params);
CN_API void cnUI_Shutdown(void);

/**
 * Call this to give the windowing system the opportunity to run its message
 * pump.  This function should be callable regardless of whether or not an
 * actual game tick occurred or will occur.
 *
 * In Win32 this might pump messages, on other systems it might poll inputs for
 * them to be distributed to other systems.
 */
CN_API void cnUI_ProcessWindowEvents(void);

typedef struct {
	CnKeyInputs keySet;
	CnMouse mouse;
} CnInput;

/**
 * Reads data out of the input system.
 */
CN_API CnInput* cnInput_Poll(void);

CN_API void cnInput_ApplyButtonMapping(const CnInput* input, CnButtonMapping* mapping);

#ifdef __cplusplus
}
#endif

#endif /* CN_UI_H */
