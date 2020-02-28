#ifndef KN_IMAGE_H
#define KN_IMAGE_H

#include <knell/kn.h>
#include <knell/memory.h>

/**
 * Images are bulk groups of pixel data stored in memory and must be fed into
 * the renderer to create a texture to use for actual drawing.
 *
 * Images are assumed to be < 4GiB
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
KN_API void ImageRGBA8_Free(ImageRGBA8* image);

#endif /* KN_IMAGE_H */
