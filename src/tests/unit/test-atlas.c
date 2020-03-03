#include <knell/test.h>

#include <knell/kn.h>
#include <knell/atlas.h>

KN_TEST_SUITE_BEGIN("atlas")
	KN_TEST_UNIT("Cannot create inappropriate texture atlases.") {
		TextureAtlas atlas;
		KN_TEST_PRECONDITION(TextureAtlas_Allocate(&atlas, (dimension2u32){ 0, 0}, 1));
		KN_TEST_PRECONDITION(TextureAtlas_Allocate(&atlas, (dimension2u32){ 1, 1}, 0));
		KN_TEST_PRECONDITION(TextureAtlas_Allocate(&atlas, (dimension2u32){ 0, 1}, 1));
		KN_TEST_PRECONDITION(TextureAtlas_Allocate(&atlas, (dimension2u32){ 1, 0}, 1));
	}

	KN_TEST_UNIT("Creating a texture atlas with many images of a single pixel subimage.") {
		TextureAtlas atlas;
		dimension2u32 subImageSize = { .width = 1, .height = 1 };
		TextureAtlas_Allocate(&atlas, subImageSize, 1);
		KN_TEST_ASSERT_EQ_U32(1, atlas.gridSizePixels.height);
		KN_TEST_ASSERT_EQ_U32(1, atlas.gridSizePixels.width);
		TextureAtlas_Free(&atlas);

		TextureAtlas_Allocate(&atlas, subImageSize, 16);
		KN_TEST_ASSERT_EQ_U32(4, atlas.gridSizePixels.height);
		KN_TEST_ASSERT_EQ_U32(4, atlas.gridSizePixels.width);
		TextureAtlas_Free(&atlas);

		TextureAtlas_Allocate(&atlas, subImageSize, 64);
		KN_TEST_ASSERT_EQ_U32(8, atlas.gridSizePixels.height);
		KN_TEST_ASSERT_EQ_U32(8, atlas.gridSizePixels.width);

		KN_TEST_ASSERT_EQ_U32(8, atlas.backingSizePixels.height);
		KN_TEST_ASSERT_EQ_U32(8, atlas.backingSizePixels.width);
		TextureAtlas_Free(&atlas);
	}

	KN_TEST_UNIT("Creating a texture atlas with many images of a larger subimage.") {
		TextureAtlas atlas;
		dimension2u32 largerSubImage = { .width = 8, .height = 16 };
		TextureAtlas_Allocate(&atlas, largerSubImage, 4);
		KN_TEST_ASSERT_EQ_U32(2, atlas.gridSizePixels.height);
		KN_TEST_ASSERT_EQ_U32(2, atlas.gridSizePixels.width);

		KN_TEST_ASSERT_EQ_U32(32, atlas.backingSizePixels.height);
		KN_TEST_ASSERT_EQ_U32(16, atlas.backingSizePixels.width);
		TextureAtlas_Free(&atlas);
	}

	KN_TEST_UNIT("SubImage start location.") {
		TextureAtlas atlas;
		dimension2u32 subImageSize = { .width = 2, .height = 3 };

		// Always forms a square TextureAtlas.
		TextureAtlas_Allocate(&atlas, subImageSize, 6);
		KN_TEST_ASSERT_EQ_U32(6, atlas.backingSizePixels.width);
		KN_TEST_ASSERT_EQ_U32(9, atlas.backingSizePixels.height);

		RowColu32 rc;
		rc = TextureAtlas_SubImageGrid(&atlas, 0);
		KN_TEST_ASSERT_EQ_U32(0, rc.col);
		KN_TEST_ASSERT_EQ_U32(0, rc.row);

		rc = TextureAtlas_SubImageGrid(&atlas, 1);
		KN_TEST_ASSERT_EQ_U32(1, rc.col);
		KN_TEST_ASSERT_EQ_U32(0, rc.row);

		rc = TextureAtlas_SubImageGrid(&atlas, 2);
		KN_TEST_ASSERT_EQ_U32(2, rc.col);
		KN_TEST_ASSERT_EQ_U32(0, rc.row);

		rc = TextureAtlas_SubImageGrid(&atlas, 3);
		KN_TEST_ASSERT_EQ_U32(0, rc.col);
		KN_TEST_ASSERT_EQ_U32(1, rc.row);

		rc = TextureAtlas_SubImageGrid(&atlas, 4);
		KN_TEST_ASSERT_EQ_U32(1, rc.col);
		KN_TEST_ASSERT_EQ_U32(1, rc.row);

		rc = TextureAtlas_SubImageGrid(&atlas, 5);
		KN_TEST_ASSERT_EQ_U32(2, rc.col);
		KN_TEST_ASSERT_EQ_U32(1, rc.row);
	}

KN_TEST_SUITE_END
