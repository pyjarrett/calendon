#include <knell/test.h>

#include <knell/kn.h>
#include <knell/assets.h>
#include <knell/log.h>

KN_TEST_SUITE_BEGIN("assets")
	KN_TEST_UNIT("Resource paths") {
		Log_Init();
		Assets_Init("assets");

		PathBuffer expected;
		PathBuffer_Create(&expected, "assets");
		PathBuffer_Join(&expected, "shaders/glsl/sample.glsl");

		PathBuffer actual;
		Assets_PathBufferFor("shaders/glsl/sample.glsl", &actual);
		KN_TEST_ASSERT_EQ_STR(expected.str, actual.str);

		Assets_Shutdown();
		Log_Shutdown();
	}

KN_TEST_SUITE_END
