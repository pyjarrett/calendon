#include <knell/test.h>

#include <knell/kn.h>
#include <knell/image.h>

KN_TEST_SUITE_BEGIN("image")
	KN_TEST_UNIT("Cannot create inappropriate texture atlases.") {
		ImageRGBA8 image;
		ImageRGBA8_AllocateSized(&image, (dimension2u32) { 8, 16 });
		ImageRGBA8_GetPixelRowCol(&image, (RowColu32) { .row = 0, .col = 0 });
	}

KN_TEST_SUITE_END
