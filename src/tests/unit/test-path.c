#include <knell/test.h>

#include <knell/path.h>

#include <stdlib.h>
#include <stdio.h>

KN_TEST_SUITE_BEGIN("Path") {
	KN_TEST_UNIT("PathBuffer Make") {
		PathBuffer pathFromCDisk;
			PathBuffer_Set(&pathFromCDisk, "C:/workshop/knell/assets");
		KN_TEST_ASSERT_EQ_STR("C:/workshop/knell/assets", pathFromCDisk.str);
	}

	KN_TEST_UNIT("PathBuffer Join") {
		PathBuffer pathFromCDisk;
			PathBuffer_Set(&pathFromCDisk, "C:/workshop/knell/assets");
		PathBuffer_Join(&pathFromCDisk, "glsl/shaders");
		KN_TEST_ASSERT_EQ_STR("C:/workshop/knell/assets/glsl/shaders", pathFromCDisk.str);
	}

	KN_TEST_UNIT("Path Add") {
		PathBuffer path;
		PathBuffer_Set(&path, "C:/Documents and Settings/Some Person/");
		const char* longSubpath = "Some Person/My Programs/Another Level of Organization/"
			"I am so organized / Path Names are Long / This Path is Getting Really Really Long/"
			"We are almost to 200 characters / I need to get this test to fail/Another Directory";
		PathBuffer_Join(&path, longSubpath);
		
	}
}

KN_TEST_SUITE_END
