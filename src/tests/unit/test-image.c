#include <calendon/test.h>

#include <calendon/cn.h>
#include <calendon/image.h>

CN_TEST_SUITE_BEGIN("image")
	CN_TEST_UNIT("Cannot create inappropriate texture atlases.") {
		CnImageRGBA8 image;
		cnImageRGBA8_AllocateSized(&image, (CnDimension2u32) { 8, 16 });
		cnImageRGBA8_GetPixelRowCol(&image, (CnRowColu32) { .row = 0, .col = 0 });
	}

CN_TEST_SUITE_END
