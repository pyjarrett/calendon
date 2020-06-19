#include <calendon/test.h>

#include <calendon/cn.h>

#include <calendon/compat-sdl.h>
#include <calendon/input-button-mapping.h>
#include <calendon/input-digital-button.h>

CN_TEST_SUITE_BEGIN("ButtonMapping")

	CN_TEST_UNIT("IDs unmapped at start") {
		CnButtonMapping mapping;
		cnButtonMapping_Clear(&mapping);
		CN_TEST_ASSERT_FALSE(cnButtonMapping_IsMapped(&mapping, SDLK_SPACE));
		CN_TEST_ASSERT_FALSE(cnButtonMapping_IsMapped(&mapping, SDLK_a));
	}

	CN_TEST_UNIT("Mapping buttons") {
		CnButtonMapping mapping;
		cnButtonMapping_Clear(&mapping);

		CN_TEST_ASSERT_FALSE(cnButtonMapping_IsMapped(&mapping, SDLK_SPACE));
		CN_TEST_ASSERT_FALSE(cnButtonMapping_IsMapped(&mapping, SDLK_a));

		CN_TEST_ASSERT_TRUE(cnButtonMapping_LookUp(&mapping, SDLK_SPACE) == NULL);
		CN_TEST_ASSERT_TRUE(cnButtonMapping_LookUp(&mapping, SDLK_a) == NULL);

		CnDigitalButton aButton;
		CnDigitalButton spaceButton;
		cnButtonMapping_Map(&mapping, SDLK_SPACE, &spaceButton);
		cnButtonMapping_Map(&mapping, SDLK_a, &aButton);

		CN_TEST_ASSERT_TRUE(cnButtonMapping_IsMapped(&mapping, SDLK_SPACE));
		CN_TEST_ASSERT_TRUE(cnButtonMapping_IsMapped(&mapping, SDLK_a));

		CN_TEST_ASSERT_TRUE(cnButtonMapping_LookUp(&mapping, SDLK_SPACE) == &spaceButton);
		CN_TEST_ASSERT_TRUE(cnButtonMapping_LookUp(&mapping, SDLK_a) == &aButton);
	}

	CN_TEST_UNIT("Unable to map from or to a null button.") {
		CnButtonMapping mapping;
		cnButtonMapping_Clear(&mapping);
		CN_TEST_PRECONDITION(cnButtonMapping_Map(&mapping, SDLK_SPACE, CN_DIGITAL_BUTTON_INVALID));

		CnDigitalButton aButton;
		CN_TEST_PRECONDITION(cnButtonMapping_Map(&mapping, CN_PHYSICAL_BUTTON_ID_INVALID, &aButton));
	}

CN_TEST_SUITE_END
