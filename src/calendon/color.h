#ifndef CN_COLOR_H
#define CN_COLOR_H

#include <calendon/cn.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
} CnRGBA8u;

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} CnRGB8u;

/**
 * An opaque color is one which cannot be seen through.
 */
typedef struct {
	float red;
	float green;
	float blue;
} CnOpaqueColor;

CN_API CnOpaqueColor cnOpaqueColor_MakeRGBf(float red, float green, float blue);
CN_API CnOpaqueColor cnOpaqueColor_MakeRGBu8(uint8_t red, uint8_t green, uint8_t blue);

#ifdef __cplusplus
}
#endif

#endif /* CN_COLOR_H */

