#include <knell/image.h>

#include <knell/assets-fileio.h>
#include <knell/compat-spng.h>
#include <knell/log.h>

extern LogHandle LogSysAssets;

bool Image_Load(ImagePixels* image, const char* fileName)
{
	KN_ASSERT(image != NULL, "Cannot load data into a null image.");
	KN_ASSERT(fileName != NULL, "Cannot load an image with a null file name.");
	DynamicBuffer fileBuffer;

	if (!Assets_ReadFile(fileName, KN_FILE_TYPE_BINARY, &fileBuffer)) {
		KN_WARN(LogSysAssets, "Unable to load image from %s", fileName);
		return false;
	}

	spng_ctx* pngContext = spng_ctx_new(0);
	spng_set_png_buffer(pngContext, fileBuffer.contents, fileBuffer.size);

	uint32_t format = SPNG_FMT_RGBA8;
	size_t decodedSize = 0;
	spng_decoded_image_size(pngContext, format, &decodedSize);

	Mem_Allocate(&image->pixels, (uint32_t)decodedSize);
	image->pixels.size = (uint32_t)decodedSize;

	spng_decode_image(pngContext, (uint8_t*)image->pixels.contents, image->pixels.size, format, 0);

	struct spng_ihdr header;
	spng_get_ihdr(pngContext, &header);
	spng_ctx_free(pngContext);

	image->width = header.width;
	image->height = header.height;

	KN_TRACE(LogSysAssets, "Loading image: %s", fileName);
	KN_TRACE(LogSysAssets, "Image size %d, %d", header.width, header.height);
	KN_TRACE(LogSysAssets, "Output size: %llu", decodedSize);
	KN_TRACE(LogSysAssets, "Input fileContents size: %d", fileBuffer.size);

    Mem_Free(&fileBuffer);

	return true;
}
