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
		KN_TEST_ASSERT_EQ_U32(1, atlas.gridSize.height);
		KN_TEST_ASSERT_EQ_U32(1, atlas.gridSize.width);
		TextureAtlas_Free(&atlas);

		TextureAtlas_Allocate(&atlas, subImageSize, 16);
		KN_TEST_ASSERT_EQ_U32(4, atlas.gridSize.height);
		KN_TEST_ASSERT_EQ_U32(4, atlas.gridSize.width);
		TextureAtlas_Free(&atlas);

		TextureAtlas_Allocate(&atlas, subImageSize, 64);
		KN_TEST_ASSERT_EQ_U32(8, atlas.gridSize.height);
		KN_TEST_ASSERT_EQ_U32(8, atlas.gridSize.width);

		KN_TEST_ASSERT_EQ_U32(8, atlas.backingSize.height);
		KN_TEST_ASSERT_EQ_U32(8, atlas.backingSize.width);
		TextureAtlas_Free(&atlas);
	}

	KN_TEST_UNIT("Creating a texture atlas with many images of a larger subimage.") {
		TextureAtlas atlas;
		dimension2u32 largerSubImage = { .width = 8, .height = 16 };
		TextureAtlas_Allocate(&atlas, largerSubImage, 4);
		KN_TEST_ASSERT_EQ_U32(2, atlas.gridSize.height);
		KN_TEST_ASSERT_EQ_U32(2, atlas.gridSize.width);

		KN_TEST_ASSERT_EQ_U32(32, atlas.backingSize.height);
		KN_TEST_ASSERT_EQ_U32(16, atlas.backingSize.width);
		TextureAtlas_Free(&atlas);
	}

KN_TEST_SUITE_END
