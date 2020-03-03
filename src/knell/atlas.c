#include "atlas.h"

#include <knell/log.h>

#include <math.h>

KN_TEST_API void TextureAtlas_Allocate(TextureAtlas* ta, dimension2u32 subImageSize, uint32_t numImages) {
	KN_ASSERT(ta != NULL, "Cannot allocate a NULL texture atlas.");
	KN_ASSERT(subImageSize.width > 0, "Cannot create a texture atlas with a zero width sub image.");
	KN_ASSERT(subImageSize.height > 0, "Cannot create a texture atlas with a zero height sub image.");
	KN_ASSERT(numImages > 0, "Cannot create a texture atlas for zero images.");
	ta->usedImages = 0;
	ta->totalImages = numImages;
	ta->subImageSizePixels = subImageSize;
	ta->gridSizePixels = (dimension2u32) { ceil(sqrt(numImages)), ceil(sqrt(numImages)) };
	ta->backingSizePixels = (dimension2u32) { ta->gridSizePixels.width * subImageSize.width,
		ta->gridSizePixels.height * subImageSize.height };

	KN_TRACE(LogSysMain, "TextureAtlas size (%" PRIu32 ", %" PRIu32 ")", ta->backingSizePixels.width, ta->backingSizePixels.height);
	ImageRGBA8_AllocateSized(&ta->image, ta->backingSizePixels);
}

KN_TEST_API void TextureAtlas_Free(TextureAtlas* ta)
{
	KN_ASSERT(ta != NULL, "Cannot free a NULL texture atlas.");
	ImageRGBA8_Free(&ta->image);
}

KN_TEST_API RowColu32 TextureAtlas_SubImageGrid(TextureAtlas* ta, uint32_t subImageId)
{
	const uint32_t row = (uint32_t)(subImageId / ta->gridSizePixels.width);
	const uint32_t col = subImageId % ta->gridSizePixels.width;
	return (RowColu32) { row, col };
}

KN_TEST_API uint32_t ImageRGBA8_offsetForRowCol(ImageRGBA8* image, RowColu32 rowCol, bool flip)
{
	KN_ASSERT(image != NULL, "Cannot get an offset into a null ImageRGBA8.");
	if (flip) {
		return rowCol.row + (image->height - rowCol.col - 1) * image->width;
	}
	else {
		return rowCol.row + rowCol.col * image->width;
	}
}

// Assumes both textures are unflipped.
KN_TEST_API uint32_t TextureAtlas_Insert(TextureAtlas* ta, ImageRGBA8* subImage) {
	KN_ASSERT(subImage != NULL, "Cannot add a null image to a texture atlas.");
	KN_ASSERT(ta->usedImages < ta->totalImages, "TextureAtlas is full.");

	// Find the (row, col) of the image within the texture atlas.
	const RowColu32 cell = TextureAtlas_SubImageGrid(ta, ta->usedImages);

	KN_TRACE(LogSysMain, "Loading subimage: row %" PRIu32 " col %" PRIu32, cell.row, cell.col);

	// The "real" offset within the image is the number of completed rows to get
	// to the row, and the number of columns left to get there.
	const uint32_t destStart = cell.row * ta->backingSizePixels.width * subImage->height + cell.col * subImage->width;

	const uint32_t bytesPerPixel = 4; // RGBA
	for (uint32_t y = 0; y < subImage->height; ++y) {
		for (uint32_t x = 0; x < subImage->width; ++x) {
//			KN_TRACE(LogSysMain, "Writing pixel (%" PRIu32 " , %" PRIu32 ")", x, y);
			const uint32_t srcOffset = x + y * subImage->width;

			// Destination rows cause offset shifts of the whole of the backing width.
			const uint32_t destOffset = destStart + x + y * ta->backingSizePixels.width;

			KN_ASSERT(bytesPerPixel * destOffset < ta->image.pixels.size, "Writing off the edge of the image.");
			KN_ASSERT(bytesPerPixel * srcOffset < subImage->pixels.size, "Reading off the edge of the image.");

			uint32_t* dest = (uint32_t*)&ta->image.pixels.contents[bytesPerPixel*destOffset];
			uint32_t* src = (uint32_t*)&subImage->pixels.contents[bytesPerPixel*srcOffset];
			*dest = *src;

			if (*src == 0 || *src == 0xFFFFFFFF) {
			}
			else {
				KN_TRACE(LogSysMain, "Unexpected subtexture value: %X", *src);
			}

//			// Copy the pixel.
//			memcpy(&ta->image.pixels.contents[bytesPerPixel*destOffset],
//				   &subImage->pixels.contents[bytesPerPixel*srcOffset],
//				   bytesPerPixel);
		}
	}

//	for (uint32_t i = 0; i < ta->image.pixels.size; i += 4) {
//		if (i % (4 * ta->image.width) == 0) printf("\n");
//		if (ta->image.pixels.contents[i] > 0) printf("X");
//	}
	return ta->usedImages++;
}
