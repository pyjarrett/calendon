#ifndef KN_IMAGE_H
#define KN_IMAGE_H

#include <knell/kn.h>
#include <knell/memory.h>

typedef struct {
	DynamicBuffer pixels;

	/*
	 * This imposes a limit of ~4 million pixels per side.  There probably
	 * shouldn't be any levels even of that size.
	 */
	uint32_t width, height;
} ImagePixels;

bool Image_Load(ImagePixels* image, const char* fileName);

#endif /* KN_IMAGE_H */
