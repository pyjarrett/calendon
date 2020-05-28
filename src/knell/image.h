#ifndef KN_IMAGE_H
#define KN_IMAGE_H

#include <knell/kn.h>

#include <knell/math2.h>
#include <knell/memory.h>
#include <knell/row_col.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Images are bulk groups of pixel data stored in memory and must be fed into
 * the renderer to create a texture to use for actual drawing.
 *
 * Images are assumed to be < 4GiB.
 *
 * Note that most images are read with Y=0 being the top row and the Y-axis
 * going downwards.
 */
typedef struct {
	DynamicBuffer pixels;

	/*
	 * This imposes a limit of ~4 million pixels per side.  There probably
	 * shouldn't be any game levels even of that size.
	 */
	uint32_t width, height;
} ImageRGBA8;

KN_API bool ImageRGBA8_Allocate(ImageRGBA8* image, const char* fileName);
KN_API bool ImageRGBA8_AllocateSized(ImageRGBA8* image, Dimension2u32 size);
KN_API void ImageRGBA8_Free(ImageRGBA8* image);
KN_API void ImageRGBA8_Flip(ImageRGBA8* image);
KN_API void ImageRGBA8_ClearRGBA(ImageRGBA8* image, uint8_t r, uint8_t b, uint8_t g, uint8_t a);
KN_API uint32_t ImageRGBA8_GetPixelRowCol(ImageRGBA8* image, RowColu32 rowCol);

#ifdef __cplusplus
}
#endif

#endif /* KN_IMAGE_H */
