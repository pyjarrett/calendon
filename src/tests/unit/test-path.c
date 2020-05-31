#include <calendon/test.h>

#include <calendon/path.h>

#include <stdlib.h>
#include <stdio.h>

CN_TEST_SUITE_BEGIN("Path") {
	CN_TEST_UNIT("CnPathBuffer Make") {
		CnPathBuffer pathFromCDisk;
			cnPathBuffer_Set(&pathFromCDisk, "C:/workshop/calendon/assets");
		CN_TEST_ASSERT_EQ_STR("C:/workshop/calendon/assets", pathFromCDisk.str);
	}

	CN_TEST_UNIT("CnPathBuffer Join") {
		CnPathBuffer pathFromCDisk;
			cnPathBuffer_Set(&pathFromCDisk, "C:/workshop/calendon/assets");
			cnPathBuffer_Join(&pathFromCDisk, "glsl/shaders");
		CN_TEST_ASSERT_EQ_STR("C:/workshop/calendon/assets/glsl/shaders", pathFromCDisk.str);
	}

	CN_TEST_UNIT("Path Add") {
		CnPathBuffer path;
			cnPathBuffer_Set(&path, "C:/Documents and Settings/Some Person/");
		const char* longSubpath = "Some Person/My Programs/Another Level of Organization/"
			"I am so organized / Path Names are Long / This Path is Getting Really Really Long/"
			"We are almost to 200 characters / I need to get this test to fail/Another Directory";
			cnPathBuffer_Join(&path, longSubpath);
		
	}
}

CN_TEST_SUITE_END
