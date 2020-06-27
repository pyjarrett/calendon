#include <calendon/test.h>

#include <calendon/cn.h>
#include <calendon/aspect-ratio.h>

bool cnDimension2u32_Equal(CnDimension2u32 left, CnDimension2u32 right)
{
	return left.width == right.width && left.height == right.height;
}

CN_TEST_SUITE_BEGIN("aspect ratio")
	CN_TEST_UNIT("same fits same") {
		CnDimension2u32 source = (CnDimension2u32) { .width = 1024, .height = 768};
		CnDimension2u32 destination = (CnDimension2u32) { .width = 1024, .height = 768 };

		CN_TEST_ASSERT_TRUE(cnDimension2u32_Equal(source, destination));
	}
CN_TEST_SUITE_END
