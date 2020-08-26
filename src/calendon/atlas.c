#include "atlas.h"

#include <calendon/log.h>

#include <math.h>

void cnTextureAtlas_Allocate(CnTextureAtlas* ta, CnDimension2u32 subImageSize, uint32_t numImages) {
	CN_ASSERT(ta != NULL, "Cannot allocate a NULL texture atlas.");
	CN_ASSERT(subImageSize.width > 0, "Cannot create a texture atlas with a zero width sub image.");
	CN_ASSERT(subImageSize.height > 0, "Cannot create a texture atlas with a zero height sub image.");
	CN_ASSERT(numImages > 0, "Cannot create a texture atlas for zero images.");
	ta->usedImages = 0;
	ta->totalImages = numImages;
	ta->subImageSizePixels = subImageSize;
	ta->gridSize = (CnDimension2u32) { (uint32_t)ceil(sqrt(numImages)), (uint32_t)ceil(sqrt(numImages)) };
	ta->backingSizePixels = (CnDimension2u32) { ta->gridSize.width * subImageSize.width,
		ta->gridSize.height * subImageSize.height };

	CN_TRACE(LogSysMain, "CnTextureAtlas size (%" PRIu32 ", %" PRIu32 ")", ta->backingSizePixels.width, ta->backingSizePixels.height);
	cnImageRGBA8_AllocateSized(&ta->image, ta->backingSizePixels);
}

void cnTextureAtlas_Free(CnTextureAtlas* ta)
{
	CN_ASSERT(ta != NULL, "Cannot free a NULL texture atlas.");
	cnImageRGBA8_Free(&ta->image);
}

CnRowColu32 cnTextureAtlas_SubImageGrid(CnTextureAtlas* ta, uint32_t subImageId)
{
	const uint32_t row = (uint32_t)(subImageId / ta->gridSize.width);
	const uint32_t col = subImageId % ta->gridSize.width;
	return (CnRowColu32) { row, col };
}

uint32_t cnImageRGBA8_OffsetForRowCol(CnImageRGBA8* image, CnRowColu32 rowCol, bool flip)
{
	CN_ASSERT(image != NULL, "Cannot get an offset into a null CnImageRGBA8.");
	if (flip) {
		return rowCol.row + (image->height - rowCol.col - 1) * image->width;
	}
	else {
		return rowCol.row + rowCol.col * image->width;
	}
}

// Assumes both textures are unflipped.
uint32_t cnTextureAtlas_Insert(CnTextureAtlas* ta, CnImageRGBA8* subImage) {
	CN_ASSERT(subImage != NULL, "Cannot add a null image to a texture atlas.");
	CN_ASSERT(ta->usedImages < ta->totalImages, "CnTextureAtlas is full.");

	// Find the (row, col) of the image within the texture atlas.
	CnRowColu32 cell = cnTextureAtlas_SubImageGrid(ta, ta->usedImages);

	// TODO: COMPLETE HACK TO GET IT TO WORK.
	cell.row = ta->gridSize.height - cell.row - 1;

//	CN_TRACE(LogSysMain, "Loading subimage: row %" PRIu32 " col %" PRIu32, cell.row, cell.col);

	// The "real" offset within the image is the number of completed rows to get
	// to the row, and the number of columns left to get there.
	const uint32_t destStart = cell.row * ta->backingSizePixels.width * subImage->height + cell.col * subImage->width;

	const uint32_t bytesPerPixel = 4; // RGBA
	for (uint32_t y = 0; y < subImage->height; ++y) {
		for (uint32_t x = 0; x < subImage->width; ++x) {
//			CN_TRACE(LogSysMain, "Writing pixel (%" PRIu32 " , %" PRIu32 ")", x, y);
			const uint32_t srcOffset = x + y * subImage->width;

			// Destination rows cause offset shifts of the whole of the backing width.
			const uint32_t destOffset = destStart + x + y * ta->backingSizePixels.width;

			CN_ASSERT(bytesPerPixel * destOffset < ta->image.pixels.size, "Writing off the edge of the image.");
			CN_ASSERT(bytesPerPixel * srcOffset < subImage->pixels.size, "Reading off the edge of the image.");

			uint32_t* dest = (uint32_t*)&ta->image.pixels.contents[bytesPerPixel*destOffset];
			uint32_t* src = (uint32_t*)&subImage->pixels.contents[bytesPerPixel*srcOffset];
			*dest = *src;

			CN_ASSERT(*src == 0 || *src == 0xFFFFFFFF, "Unexpected subtexture value: %X", *src);
		}
	}
	return ta->usedImages++;
}

void cnTextureAtlas_TexCoordForSubImage(CnTextureAtlas* ta, CnFloat2* output, uint32_t subImageId)
{
	CN_ASSERT(ta != NULL, "Cannot get subtexture coordiantes for a null texture atlas.");
	CN_ASSERT(output != NULL, "Cannot write subtexture coordinates to a null location.");

	// TODO: Should this be used images or total images?
	CN_ASSERT(subImageId < ta->totalImages, "SubImage %" PRIu32 " is outside of "
		"range of texture atlas: %" PRIu32, subImageId, ta->totalImages);

	// TODO: Look into using normalized vertex attributes and integers here instead.
	const float dx = 1.0f / ta->gridSize.height;
	const float dy = 1.0f / ta->gridSize.height;

	const CnRowColu32 rowCol = cnTextureAtlas_SubImageGrid(ta, subImageId);
	output[0] = cnFloat2_Make(rowCol.col * dx, rowCol.row * dy);
	output[1] = cnFloat2_Make((rowCol.col + 1.0f) * dx, rowCol.row * dy);
	output[2] = cnFloat2_Make(rowCol.col * dx, (rowCol.row + 1.0f) * dy);
	output[3] = cnFloat2_Make((rowCol.col + 1.0f) * dx, (rowCol.row + 1.0f) * dy);
}
