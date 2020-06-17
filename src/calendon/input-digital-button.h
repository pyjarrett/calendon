#ifndef CN_INPUT_DIGITAL_BUTTON_H
#define CN_INPUT_DIGITAL_BUTTON_H

#include <calendon/cn.h>

typedef enum {
	CnDigitalButtonStateDown = 0,
	CnDigitalButtonStateUp = 1
} CnDigitalButtonState;

/**
 * A digital button with two different states: pressed (down) and
 * released (up).  This button is an abstract logical concept of
 * a digital button, so it has no concept of being assigned to a specific
 * button, such as "A", "Enter", or "Escape".
 */
typedef struct {
	CnDigitalButtonState state;
} CnDigitalButton;

CN_API bool cnDigitalButton_Set(CnDigitalButton* button, CnDigitalButtonState state);

CN_API bool cnDigitalButton_IsDown(CnDigitalButton* button);
CN_API bool cnDigitalButton_IsUp(CnDigitalButton* button);

CN_API void cnDigitalButton_Press(CnDigitalButton* button);
CN_API void cnDigitalButton_Release(CnDigitalButton* button);

#endif /* CN_INPUT_DIGITAL_BUTTON_H */
