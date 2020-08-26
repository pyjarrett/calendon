#ifndef CN_IMAGE_H
#define CN_IMAGE_H

#include <calendon/cn.h>

#include <calendon/math2.h>
#include <calendon/memory.h>
#include <calendon/row-col.h>

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
	CnDynamicBuffer pixels;

	/*
	 * This imposes a limit of ~4 million pixels per side.  There probably
	 * shouldn't be any game levels even of that size.
	 */
	uint32_t width, height;
} CnImageRGBA8;

CN_API bool          cnImageRGBA8_Allocate(CnImageRGBA8* image, const char* fileName);
CN_API bool          cnImageRGBA8_AllocateSized(CnImageRGBA8* image, CnDimension2u32 size);
CN_API void          cnImageRGBA8_Free(CnImageRGBA8* image);
CN_API void          cnImageRGBA8_Flip(CnImageRGBA8* image);
CN_API void          cnImageRGBA8_ClearRGBA(CnImageRGBA8* image, uint8_t r, uint8_t b, uint8_t g, uint8_t a);
CN_API uint32_t      cnImageRGBA8_GetPixelRowCol(CnImageRGBA8* image, CnRowColu32 rowCol);
CN_TEST_API uint32_t cnImageRGBA8_OffsetForRowCol(CnImageRGBA8* image, CnRowColu32 rowCol, bool flip);

#ifdef __cplusplus
}
#endif

#endif /* CN_IMAGE_H */
