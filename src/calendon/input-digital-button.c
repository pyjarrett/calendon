#include "input-digital-button.h"

bool cnDigitalButton_Set(CnDigitalButton* button, CnDigitalButtonState state)
{
	CN_ASSERT_PTR(button);
	button->state = state;
	return true;
}

bool cnDigitalButton_IsDown(CnDigitalButton* button)
{
	CN_ASSERT_PTR(button);
	return button->state == CnDigitalButtonStateDown;
}

bool cnDigitalButton_IsUp(CnDigitalButton* button)
{
	CN_ASSERT_PTR(button);
	return button->state == CnDigitalButtonStateUp;
}

void cnDigitalButton_Press(CnDigitalButton* button)
{
	CN_ASSERT_PTR(button);
	button->state = CnDigitalButtonStateDown;
}

void cnDigitalButton_Release(CnDigitalButton* button)
{
	CN_ASSERT_PTR(button);
	button->state = CnDigitalButtonStateUp;
}
