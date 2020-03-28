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
}

KN_TEST_SUITE_END
