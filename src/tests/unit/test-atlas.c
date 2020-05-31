#include <calendon/test.h>

#include <calendon/cn.h>
#include <calendon/atlas.h>

CN_TEST_SUITE_BEGIN("atlas")
	CN_TEST_UNIT("Cannot create inappropriate texture atlases.") {
		CnTextureAtlas atlas;
		CN_TEST_PRECONDITION(cnTextureAtlas_Allocate(&atlas, (CnDimension2u32) { 0, 0 }, 1));
		CN_TEST_PRECONDITION(cnTextureAtlas_Allocate(&atlas, (CnDimension2u32) { 1, 1 }, 0));
		CN_TEST_PRECONDITION(cnTextureAtlas_Allocate(&atlas, (CnDimension2u32) { 0, 1 }, 1));
		CN_TEST_PRECONDITION(cnTextureAtlas_Allocate(&atlas, (CnDimension2u32) { 1, 0 }, 1));
	}

	CN_TEST_UNIT("Creating a texture atlas with many images of a single pixel subimage.") {
		CnTextureAtlas atlas;
		CnDimension2u32 subImageSize = { .width = 1, .height = 1 };
		cnTextureAtlas_Allocate(&atlas, subImageSize, 1);
		CN_TEST_ASSERT_EQ_U32(1, atlas.gridSize.height);
		CN_TEST_ASSERT_EQ_U32(1, atlas.gridSize.width);
		cnTextureAtlas_Free(&atlas);

		cnTextureAtlas_Allocate(&atlas, subImageSize, 16);
		CN_TEST_ASSERT_EQ_U32(4, atlas.gridSize.height);
		CN_TEST_ASSERT_EQ_U32(4, atlas.gridSize.width);
		cnTextureAtlas_Free(&atlas);

		cnTextureAtlas_Allocate(&atlas, subImageSize, 64);
		CN_TEST_ASSERT_EQ_U32(8, atlas.gridSize.height);
		CN_TEST_ASSERT_EQ_U32(8, atlas.gridSize.width);

		CN_TEST_ASSERT_EQ_U32(8, atlas.backingSizePixels.height);
		CN_TEST_ASSERT_EQ_U32(8, atlas.backingSizePixels.width);
		cnTextureAtlas_Free(&atlas);
	}

	CN_TEST_UNIT("Creating a texture atlas with many images of a larger subimage.") {
		CnTextureAtlas atlas;
		CnDimension2u32 largerSubImage = { .width = 8, .height = 16 };
		cnTextureAtlas_Allocate(&atlas, largerSubImage, 4);
		CN_TEST_ASSERT_EQ_U32(2, atlas.gridSize.height);
		CN_TEST_ASSERT_EQ_U32(2, atlas.gridSize.width);

		CN_TEST_ASSERT_EQ_U32(32, atlas.backingSizePixels.height);
		CN_TEST_ASSERT_EQ_U32(16, atlas.backingSizePixels.width);
		cnTextureAtlas_Free(&atlas);
	}

	CN_TEST_UNIT("SubImage start location.") {
		CnTextureAtlas atlas;
		CnDimension2u32 subImageSize = { .width = 2, .height = 3 };

		// Always forms a square CnTextureAtlas.
		cnTextureAtlas_Allocate(&atlas, subImageSize, 6);
		CN_TEST_ASSERT_EQ_U32(6, atlas.backingSizePixels.width);
		CN_TEST_ASSERT_EQ_U32(9, atlas.backingSizePixels.height);

		CnRowColu32 rc;
		rc = cnTextureAtlas_SubImageGrid(&atlas, 0);
		CN_TEST_ASSERT_EQ_U32(0, rc.col);
		CN_TEST_ASSERT_EQ_U32(0, rc.row);

		rc = cnTextureAtlas_SubImageGrid(&atlas, 1);
		CN_TEST_ASSERT_EQ_U32(1, rc.col);
		CN_TEST_ASSERT_EQ_U32(0, rc.row);

		rc = cnTextureAtlas_SubImageGrid(&atlas, 2);
		CN_TEST_ASSERT_EQ_U32(2, rc.col);
		CN_TEST_ASSERT_EQ_U32(0, rc.row);

		rc = cnTextureAtlas_SubImageGrid(&atlas, 3);
		CN_TEST_ASSERT_EQ_U32(0, rc.col);
		CN_TEST_ASSERT_EQ_U32(1, rc.row);

		rc = cnTextureAtlas_SubImageGrid(&atlas, 4);
		CN_TEST_ASSERT_EQ_U32(1, rc.col);
		CN_TEST_ASSERT_EQ_U32(1, rc.row);

		rc = cnTextureAtlas_SubImageGrid(&atlas, 5);
		CN_TEST_ASSERT_EQ_U32(2, rc.col);
		CN_TEST_ASSERT_EQ_U32(1, rc.row);
	}

	CN_TEST_UNIT("Square texture coordinate locations for sub image.") {
		CnTextureAtlas squareAtlas;
		CnDimension2u32 subImageSize = { .width = 2, .height = 2 };

		CnFloat2 texCoords[4];
		cnTextureAtlas_Allocate(&squareAtlas, subImageSize, 4);

		// First subImage.
		cnTextureAtlas_TexCoordForSubImage(&squareAtlas, texCoords, 0);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnFloat2_Make(0.0f, 0.0f), texCoords[0]) < 0.1f);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnFloat2_Make(0.5f, 0.0f), texCoords[1]) < 0.1f);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnFloat2_Make(0.0f, 0.5f), texCoords[2]) < 0.1f);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnFloat2_Make(0.5f, 0.5f), texCoords[3]) < 0.1f);

		cnTextureAtlas_TexCoordForSubImage(&squareAtlas, texCoords, 1);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnFloat2_Make(0.5f, 0.0f), texCoords[0]) < 0.1f);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnFloat2_Make(1.0f, 0.0f), texCoords[1]) < 0.1f);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnFloat2_Make(0.5f, 0.5f), texCoords[2]) < 0.1f);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnFloat2_Make(1.0f, 0.5f), texCoords[3]) < 0.1f);

		cnTextureAtlas_TexCoordForSubImage(&squareAtlas, texCoords, 2);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnFloat2_Make(0.0f, 0.5f), texCoords[0]) < 0.1f);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnFloat2_Make(0.5f, 0.5f), texCoords[1]) < 0.1f);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnFloat2_Make(0.0f, 1.0f), texCoords[2]) < 0.1f);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnFloat2_Make(0.5f, 1.0f), texCoords[3]) < 0.1f);

		cnTextureAtlas_TexCoordForSubImage(&squareAtlas, texCoords, 3);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnFloat2_Make(0.5f, 0.5f), texCoords[0]) < 0.1f);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnFloat2_Make(1.0f, 0.5f), texCoords[1]) < 0.1f);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnFloat2_Make(0.5f, 1.0f), texCoords[2]) < 0.1f);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(
			cnFloat2_Make(1.0f, 1.0f), texCoords[3]) < 0.1f);
		cnTextureAtlas_Free(&squareAtlas);
	}

CN_TEST_SUITE_END
