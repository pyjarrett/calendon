#ifndef CN_ATLAS_H
#define CN_ATLAS_H

#include <calendon/cn.h>

#include <calendon/image.h>
#include <calendon/math2.h>
#include <calendon/row-col.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A texture atlas which assumes that all images are the same size.
 */
typedef struct {
	CnImageRGBA8 image;
	uint32_t usedImages;
	uint32_t totalImages;

	/** The dimensions of each subimage making up the texture atlas. */
	CnDimension2u32 subImageSizePixels;

	/** The total size of the underlying image. */
	CnDimension2u32 backingSizePixels;

	/** The number of subimages which fit across the width and height. */
	CnDimension2u32 gridSize;
} CnTextureAtlas;

CN_TEST_API void        cnTextureAtlas_Allocate(CnTextureAtlas* ta, CnDimension2u32 subImageSize, uint32_t numImages);
CN_TEST_API void        cnTextureAtlas_Free(CnTextureAtlas* ta);
CN_TEST_API CnRowColu32 cnTextureAtlas_SubImageGrid(CnTextureAtlas* ta, uint32_t subImageId);
CN_TEST_API uint32_t    cnTextureAtlas_Insert(CnTextureAtlas* ta, CnImageRGBA8* subImage);
CN_TEST_API void        cnTextureAtlas_TexCoordForSubImage(CnTextureAtlas* ta, CnFloat2* output, uint32_t subImageId);

#ifdef __cplusplus
}
#endif

#endif /* CN_ATLAS_H */
