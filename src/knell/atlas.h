#ifndef KN_ATLAS_H
#define KN_ATLAS_H

#include <knell/kn.h>
#include <knell/image.h>
#include <knell/math2.h>

typedef struct {
	ImageRGBA8 image;
	uint32_t usedImages;
	uint32_t totalImages;
	dimension2u32 subImageSizePixels;
	dimension2u32 backingSizePixels;

	/** The number of subimages which fit across the width and height. */
	dimension2u32 gridSize;
} TextureAtlas;

KN_TEST_API void TextureAtlas_Allocate(TextureAtlas* ta, dimension2u32 subImageSize, uint32_t numImages);
KN_TEST_API void TextureAtlas_Free(TextureAtlas* ta);
KN_TEST_API RowColu32 TextureAtlas_SubImageGrid(TextureAtlas* ta, uint32_t subImageId);
KN_TEST_API uint32_t ImageRGBA8_offsetForRowCol(ImageRGBA8* image, RowColu32 rowCol, bool flip);
KN_TEST_API uint32_t TextureAtlas_Insert(TextureAtlas* ta, ImageRGBA8* subImage);
KN_TEST_API void TextureAtlas_TexCoordForSubImage(TextureAtlas* ta, float2* output, uint32_t subImageId);

#endif /* KN_ATLAS_H */
